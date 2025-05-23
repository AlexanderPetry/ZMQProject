// Includes

#include <zmq.hpp>
#include <thread>


#include <QThread>
#include <QCoreApplication>
#include <QDataStream>
#include <QSysInfo>
#include <QSerialPortInfo>
#include <QDebug>

#include "synthservice.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

bool local = false;

int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

    SynthService service(local);
    service.receiveLoop();

    return app.exec();
}
