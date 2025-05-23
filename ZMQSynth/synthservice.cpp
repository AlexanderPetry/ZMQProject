#include "synthservice.h"
#include <thread>
#include <chrono>

// Constructor
SynthService::SynthService(bool localMode)
    : local(localMode),
      context(1),
      receiver(context, ZMQ_SUB),
      publisher(context, local ? ZMQ_PUB : ZMQ_PUSH)
{
    // Setup sockets (example addresses, adjust as needed)
    if(local) {
        publisher.bind("tcp://127.0.0.1:24041");
        receiver.bind("tcp://127.0.0.1:24042");
    } else {
        publisher.connect("tcp://benternet.pxl-ea-ict.be:24041");
        receiver.connect("tcp://benternet.pxl-ea-ict.be:24042");
    }
    receiver.setsockopt(ZMQ_SUBSCRIBE, "pynqsynth@", 10);

    logSocket = &publisher;
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
    // Your existing playNote implementation
    std::ostringstream ss;
    ss << "Playing note " << note << (status ? " down" : " up") << " at velocity " << velocity;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    // Actual MIDI/serial sending logic here
}

void SynthService::switchInstrument(int instrument) {
    std::ostringstream ss;
    ss << "Switching instrument to " << instrument;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());
    // Send to hardware...
}

void SynthService::switchEffect(int effect) {
    std::ostringstream ss;
    ss << "Switching effect to " << effect;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());
    // Send to hardware...
}

void SynthService::customInstrument(int w, int a, int d, int s, int r) {
    std::ostringstream ss;
    ss << "Custom instrument settings: " << w << " " << a << " " << d << " " << s << " " << r;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());
    // Send to hardware...
}

void SynthService::updateAll(const ClientState& state) {
    switchInstrument(state.instrument);
    switchEffect(state.effect);
    customInstrument(state.waveform, state.attack, state.decay, state.sustain, state.release);
}

void SynthService::receiveLoop() {
    zmq::message_t msg;
    while (true) {
        receiver.recv(msg);
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
