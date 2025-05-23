#ifndef SYNTHSERVICE_H
#define SYNTHSERVICE_H

#include <zmq.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <QThread>

struct ClientState {
    int instrument = 0;
    int effect = 0;
    int waveform = 0;
    int attack = 0;
    int decay = 0;
    int sustain = 0;
    int release = 0;
};

class SynthService {
public:
    SynthService(bool localMode);

    void receiveLoop();
    void heartbeat();

    void logMessage(const std::string& msg);
    void sendError(const std::string& errMsg);

    void playNote(int status, int note, int velocity);
    void switchInstrument(int instrument);
    void switchEffect(int effect);
    void customInstrument(int waveform, int attack, int decay, int sustain, int release);

    void updateAll(const ClientState& state);

private:
    bool local;
    zmq::context_t context;
    zmq::socket_t receiver;
    zmq::socket_t publisher;
    zmq::socket_t* logSocket;

    std::unordered_map<std::string, ClientState> clientStates;
    std::string lastClient;

    // Add other members/methods as needed
};

#endif
