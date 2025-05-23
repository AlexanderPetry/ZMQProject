TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += qt

QT += serialport
QT += core

DEFINES += ZMQ_STATIC

INCLUDEPATH += $$PWD/../include
LIBS += -L$$PWD/../lib -lpthread

unix {
    # Linux-specific settings
    LIBS += -L$$PWD/zmq/libzmq/src/.libs -lzmq
}

win32 {
    # Windows-specific settings
    LIBS += -L$$PWD/lib -lzmq
    LIBS += -lws2_32 -lIphlpapi
}

SOURCES += main.cpp \
    heartbeatworker.cpp \
    serialconnection.cpp \
    synthservice.cpp

HEADERS += \
    heartbeatworker.h \
    serialconnection.h \
    synthservice.h
