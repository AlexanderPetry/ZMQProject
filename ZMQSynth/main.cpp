// Includes
#include <iostream>
#include <zmq.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

bool local = 0;

std::string adress1 = "tcp://benternet.pxl-ea-ict.be:24041";
std::string adress2 = "tcp://benternet.pxl-ea-ict.be:24042";
std::string ladress1 = "tcp://127.0.0.1:24041";
std::string ladress2 = "tcp://127.0.0.1:24042";

void playNote(int note, float vel, float dur) {
    std::cout << "Playing note: note=" << note << ", vel=" << vel << ", dur=" << dur << std::endl;
}

void playChord(const std::string& chord, float vel, float dur) {
    std::cout << "Playing chord: " << chord << " vel=" << vel << " dur=" << dur << std::endl;
}

void setInstrument(const std::string& instrument) {
    std::cout << "Setting instrument: " << instrument << std::endl;
}

void setEnvelope(float a, float d, float s, float r) {
    std::cout << "Setting envelope: A=" << a << " D=" << d << " S=" << s << " R=" << r << std::endl;
}

void setVolume(float volume) {
    std::cout << "Setting volume: " << volume << std::endl;
}

void heartbeat(zmq::context_t& context) {
    zmq::socket_t sender(context, local ? ZMQ_PUB : ZMQ_PUSH);
    if (local) sender.bind(ladress1); else sender.connect(adress1);
    while (true) {
        std::string msg = "status.reply!>alive";
        zmq::message_t message(msg.begin(), msg.end());
        sender.send(message);//, zmq::send_flags::none);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_SUB);

    if (local) {
        receiver.bind(ladress2);
        std::cout << "Listening locally on " << ladress2 << std::endl;
    } else {
        receiver.connect(adress2);
        std::cout << "Connected to Benternet at " << adress2 << std::endl;
    }

    std::vector<std::string> topics = {
        "note.play?>", "chord.play?>", "instrument.set?>",
        "envelope.set?>", "volume.set?>"
    };

    for (const auto& topic : topics) {
        receiver.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    }

    std::thread heartbeat_thread(heartbeat, std::ref(context));
    heartbeat_thread.detach();

    zmq::message_t msg;
    while (true) {
        receiver.recv(&msg);
        std::string data(static_cast<char*>(msg.data()), msg.size());
        std::cout << "Received: " << data << std::endl;

        auto sep = data.find("?>");
        if (sep == std::string::npos) continue;
        std::string cmd = data.substr(0, sep + 2);
        std::string payload = data.substr(sep + 2);
        std::istringstream ss(payload);

        if (cmd == "note.play?>") {
            float f, v, d;
            ss >> f >> v >> d;
            playNote(f, v, d);
        } else if (cmd == "chord.play?>") {
            std::string chord;
            float v, d;
            ss >> chord >> v >> d;
            playChord(chord, v, d);
        } else if (cmd == "instrument.set?>") {
            std::string name;
            ss >> name;
            setInstrument(name);
        } else if (cmd == "envelope.set?>") {
            float a, d, s, r;
            ss >> a >> d >> s >> r;
            setEnvelope(a, d, s, r);
        } else if (cmd == "volume.set?>") {
            float vol;
            ss >> vol;
            setVolume(vol);
        }
    }

    return 0;
}
