#ifndef SYNTH_H
#define SYNTH_H

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
#include <QSerialPortInfo>
#include <QDebug>

#include "serialconnection.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

extern SerialConnection *sc;

extern bool local;

extern std::string adress1;
extern std::string adress2;
extern std::string ladress1;
extern std::string ladress2;

extern zmq::socket_t* logSocket;

class synth
{
public:
    synth();
    void logMessage(const std::string& msg);
    void playNote(int status, int note, int velocity);
    void switchInstrument(int instrument);
    void switchEffect(int effect);
    void customInstrument(int waveform, int attack, int decay, int sustain, int release);
    void heartbeat(zmq::context_t& context);
    void listSerialPorts();
    void play();
    void sendError(const std::string &errMsg);
};

#endif // SYNTH_H
