#include "synthservice.h"
#include "heartbeatworker.h"
#include "qdebug.h"
#include <thread>
#include <chrono>

SynthService::SynthService(bool localMode)
    : local(localMode),
      context(1),
      receiver(context, ZMQ_SUB),
      publisher(context, local ? ZMQ_PUB : ZMQ_PUSH)
{
    if(local) {
        publisher.bind("tcp://127.0.0.1:24041");
        receiver.bind("tcp://127.0.0.1:24042");
    } else {
        publisher.connect("tcp://benternet.pxl-ea-ict.be:24041");
        receiver.connect("tcp://benternet.pxl-ea-ict.be:24042");
    }
    receiver.setsockopt(ZMQ_SUBSCRIBE, "pynqsynth@", 10);

    logSocket = &publisher;

    //SynthService service(local);

    QThread* hbThread = new QThread;
    HeartbeatWorker* hbWorker = new HeartbeatWorker(context, local);
    hbWorker->moveToThread(hbThread);

    QObject::connect(hbThread, &QThread::started, hbWorker, &HeartbeatWorker::process);
    QObject::connect(hbWorker, &HeartbeatWorker::finished, hbThread, &QThread::quit);
    QObject::connect(hbWorker, &HeartbeatWorker::finished, hbWorker, &HeartbeatWorker::deleteLater);
    QObject::connect(hbThread, &QThread::finished, hbThread, &QThread::deleteLater);

    hbThread->start();

    QThread* thread = new QThread;
    sc = new SerialConnection();
    sc->moveToThread(thread);
    thread->start();
}

void SynthService::logMessage(const std::string& msg) {
    std::string formatted = "pynqsynth@synth.log!>" + msg;
    zmq::message_t message(formatted.begin(), formatted.end());
    logSocket->send(message);
}

void SynthService::sendError(const std::string& errMsg) {
    std::string err = "pynqsynth@error.report?>" + errMsg;
    zmq::message_t errorMsg(err.begin(), err.end());
    logSocket->send(errorMsg);
}

void SynthService::playNote(int status, int note, int velocity) {
    std::ostringstream ss;
    ss << "Playing note " << note << (status ? " down" : " up") << " at velocity " << velocity;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    QByteArray data;
    // Note ON: 0x90, Note OFF: 0x80
    data.append(static_cast<char>(status ? 0x90 : 0x80));
    data.append(static_cast<char>(note));
    data.append(static_cast<char>(velocity));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);
}

void SynthService::switchInstrument(int instrument) {
    std::ostringstream ss;
    ss << "Switching instrument to " << instrument;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    QByteArray data;
    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x00));
    data.append(static_cast<char>(instrument));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);
}

void SynthService::switchEffect(int effect) {
    std::ostringstream ss;
    ss << "Switching effect to " << effect;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);


    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x15));
    data.append(static_cast<char>(effect));


    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);
}

void SynthService::customInstrument(int waveform, int attack, int decay, int sustain, int release) {
    std::ostringstream ss;
    ss << "Custom instrument settings: waveform=" << waveform
       << ", attack=" << attack
       << ", decay=" << decay
       << ", sustain=" << sustain
       << ", release=" << release;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    // Waveform
    QByteArray dataWave;
    dataWave.append(static_cast<char>(0xB0));
    dataWave.append(static_cast<char>(0x10));
    dataWave.append(static_cast<char>(waveform));
    QMetaObject::invokeMethod(sc, [=]() { sc->writeData(dataWave); }, Qt::QueuedConnection);

    // Attack
    QByteArray dataAttack;
    dataAttack.append(static_cast<char>(0xB0));
    dataAttack.append(static_cast<char>(0x11));
    dataAttack.append(static_cast<char>(attack));
    QMetaObject::invokeMethod(sc, [=]() { sc->writeData(dataAttack); }, Qt::QueuedConnection);

    // Decay
    QByteArray dataDecay;
    dataDecay.append(static_cast<char>(0xB0));
    dataDecay.append(static_cast<char>(0x12));
    dataDecay.append(static_cast<char>(decay));
    QMetaObject::invokeMethod(sc, [=]() { sc->writeData(dataDecay); }, Qt::QueuedConnection);

    // Sustain
    QByteArray dataSustain;
    dataSustain.append(static_cast<char>(0xB0));
    dataSustain.append(static_cast<char>(0x13));
    dataSustain.append(static_cast<char>(sustain));
    QMetaObject::invokeMethod(sc, [=]() { sc->writeData(dataSustain); }, Qt::QueuedConnection);

    // Release
    QByteArray dataRelease;
    dataRelease.append(static_cast<char>(0xB0));
    dataRelease.append(static_cast<char>(0x14));
    dataRelease.append(static_cast<char>(release));
    QMetaObject::invokeMethod(sc, [=]() { sc->writeData(dataRelease); }, Qt::QueuedConnection);
}

void SynthService::updateAll(const ClientState& state) {
    switchInstrument(state.instrument);
    switchEffect(state.effect);
    customInstrument(state.waveform, state.attack, state.decay, state.sustain, state.release);
}

void SynthService::receiveLoop() {
    zmq::message_t msg;
    while (true) {
        receiver.recv(&msg);
        std::string data(static_cast<char*>(msg.data()), msg.size());

        if (data.find("pynqsynth@synth.log!>") == 0
            || data.find("pynqsynth@status.reply!>") == 0
            || data.find("pynqsynth@error.report?>") == 0) {
            continue; // avoid recursion
        }

        logMessage("Received: " + data);

        const std::string prefix = "pynqsynth@";
        if (data.compare(0, prefix.size(), prefix) != 0) continue;

        auto sep = data.find("?>", prefix.size());
        if (sep == std::string::npos) {
            sendError("Malformed command (missing '?>'): " + data);
            continue;
        }

        std::string cmd = data.substr(prefix.size(), sep - prefix.size() + 2);
        std::string payload = data.substr(sep + 2);

        // Format: =clientID= params...
        if (payload.size() < 3 || payload[0] != '=') {
            sendError("Malformed payload (missing client ID): " + payload);
            continue;
        }

        auto secondEq = payload.find('=', 1);
        if (secondEq == std::string::npos) {
            sendError("Malformed payload (missing second '='): " + payload);
            continue;
        }

        std::string clientID = payload.substr(1, secondEq - 1);
        std::string params = payload.substr(secondEq + 1);

        std::istringstream ss(params);
        ClientState& state = clientStates[clientID];
        lastClient = clientID;

        if (cmd == "note.play?>") {
            int status, note, velocity;
            if (!(ss >> status >> note >> velocity)) {
                sendError("Invalid parameters for note.play: " + params);
                continue;
            }
            playNote(status, note, velocity);
        }
        else if (cmd == "instrument.change?>") {
            int instrument;
            if (!(ss >> instrument)) {
                sendError("Invalid parameter for instrument.change: " + params);
                continue;
            }
            state.instrument = instrument;
            switchInstrument(instrument);
        }
        else if (cmd == "custom.instrument?>") {
            int w, a, d, s, r;
            if (!(ss >> w >> a >> d >> s >> r)) {
                sendError("Invalid parameters for custom.instrument: " + params);
                continue;
            }
            state.waveform = w;
            state.attack = a;
            state.decay = d;
            state.sustain = s;
            state.release = r;
            customInstrument(w, a, d, s, r);
        }
        else if (cmd == "effect.change?>") {
            int effect;
            if (!(ss >> effect)) {
                sendError("Invalid parameter for effect.change: " + params);
                continue;
            }
            state.effect = effect;
            switchEffect(effect);
        }
        else {
            sendError("Unknown command: " + cmd);
        }
    }
}
