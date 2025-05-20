// Includes
#include <iostream>
#include <zmq.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

#include <QThread>
#include <QCoreApplication>
#include <QDataStream>
#include <QSysInfo>

#include "serialconnection.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

SerialConnection *sc;

bool local = 0;

std::string adress1 = "tcp://benternet.pxl-ea-ict.be:24041";
std::string adress2 = "tcp://benternet.pxl-ea-ict.be:24042";
std::string ladress1 = "tcp://127.0.0.1:24041";
std::string ladress2 = "tcp://127.0.0.1:24042";

zmq::socket_t* logSocket = nullptr;

void logMessage(const std::string& msg) {
    std::string formatted = "synth.log!>" + msg;
    zmq::message_t message(formatted.begin(), formatted.end());
    logSocket->send(message);
}

void playNote(int status, int note, int velocity) {
    std::ostringstream ss;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    if(status == true)
    {
        ss << "Note " << note << " down at velocity " << velocity;
    }
    else
    {
        ss << "Note " << note << " up at velocity " << velocity;
    }

    data.append(static_cast<char>(status ? 0x90 : 0x80));
    data.append(static_cast<char>(note));
    data.append(static_cast<char>(velocity));

    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);
}

void switchInstrument(int instrument) {
    std::ostringstream ss;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);


    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x00));
    data.append(static_cast<char>(instrument));

    ss << instrument;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);
}

void customInstrument(int waveform, int attack, int decay, int sustain, int release) {
    std::ostringstream ss;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);


    data.append(static_cast<char>(0xB0)); // Valid status
    data.append(static_cast<char>(0x10)); // Controller number for waveform
    data.append(static_cast<char>(waveform));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);

    ss << waveform;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    data.clear();
    ss.clear();

    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x11));
    data.append(static_cast<char>(attack));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);

    ss <<attack << decay;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    data.clear();
    ss.clear();


    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x12));
    data.append(static_cast<char>(decay));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);

    ss <<attack << decay;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    data.clear();
    ss.clear();

    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x13));
    data.append(static_cast<char>(sustain));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);

    ss << sustain << release;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());

    data.clear();
    ss.clear();

    data.append(static_cast<char>(0xB0));
    data.append(static_cast<char>(0x14));
    data.append(static_cast<char>(release));

    QMetaObject::invokeMethod(sc, [=]() {
        sc->writeData(data);
    }, Qt::QueuedConnection);

    ss << sustain << release;
    std::cout << ss.str() << std::endl;
    logMessage(ss.str());
}


void heartbeat(zmq::context_t& context) {
    zmq::socket_t sender(context, local ? ZMQ_PUB : ZMQ_PUSH);
    if (local) sender.bind(ladress1); else sender.connect(adress1);
    while (true) {
        std::string msg = "status.reply!>alive";
        zmq::message_t message(msg.begin(), msg.end());
        sender.send(message);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_SUB);
    zmq::socket_t logger(context, local ? ZMQ_PUB : ZMQ_PUSH);
    if (local) logger.bind(ladress1); else logger.connect(adress1);
    logSocket = &logger;

    if (local) {
        receiver.bind(ladress2);
        std::cout << "Listening locally on " << ladress2 << std::endl;
    } else {
        receiver.connect(adress2);
        std::cout << "Connected to Benternet at " << adress2 << std::endl;
    }

    std::vector<std::string> topics = {
        "note.play?>", "chord.play?>", "instrument.change?>",
        "custom.instrument?>", "volume.set?>"
    };

    for (const auto& topic : topics) {
        receiver.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    }

    std::thread heartbeat_thread(heartbeat, std::ref(context));
    heartbeat_thread.detach();

    QThread* thread = new QThread;
    sc = new SerialConnection();
    sc->moveToThread(thread);
    thread->start();

    QMetaObject::invokeMethod(sc, [=]()
    {
        QString portName;

        #if defined(Q_OS_WIN)  // For Windows
                portName = "COM4";
        #elif defined(Q_OS_LINUX)  // For Linux (e.g., Raspberry Pi)
                portName = "/dev/ttyUSB0";  // Adjust this based on your setup (ttyUSB1, etc.)
        #else
                portName = "";  // Set a default or handle other platforms
        #endif

        sc->openSerialPort(portName, 115200);
    }, Qt::QueuedConnection);

    zmq::message_t msg;
    while (true) {
        receiver.recv(&msg);
        std::string data(static_cast<char*>(msg.data()), msg.size());
        std::cout << "Received: " << data << std::endl;
        logMessage("Received: " + data);

        auto sep = data.find("?>");
        if (sep == std::string::npos) continue;
        std::string cmd = data.substr(0, sep + 2);
        std::string payload = data.substr(sep + 2);
        std::istringstream ss(payload);

        if (cmd == "note.play?>")
        {
            int f, v, d;
            ss >> f >> v >> d;
            playNote(f, v, d);
        }
        else if(cmd == "instrument.change?>")
        {
            int d;
            ss >> d;
            switchInstrument(d);
        }
        else if (cmd == "custom.instrument?>")
        {
            int w, a, d, s, r;
            ss >> w >> a >> d >> s >> r;
            customInstrument(w, a, d, s, r);
        }
    }

    return app.exec();
}
