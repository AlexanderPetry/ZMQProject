// Includes
#include <zmq.hpp>
#include <QThread>
#include <QCoreApplication>
#include <QDataStream>
#include <QSysInfo>
#include <QSerialPortInfo>
#include <QDebug>
#include "synth.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    synth *s = new synth();
    s->play();

    return app.exec();
}
