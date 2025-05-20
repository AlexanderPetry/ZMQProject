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
    LIBS += -L$$PWD/src/.libs -lzmq   # Adjust path if necessary
}

win32 {
    # Windows-specific settings
    LIBS += -L$$PWD/lib -lzmq.lib
    # If you have the Windows-specific libraries like ws2_32 and Iphlpapi
    LIBS += -lws2_32 -lIphlpapi
}

SOURCES += main.cpp \
    serialconnection.cpp

HEADERS += \
    serialconnection.h
