TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += qt

QT += core
QT += gui
QT += widgets

DEFINES += ZMQ_STATIC

LIBS += -L$$PWD/../lib -lws2_32 -lpthread -lIphlpapi -lzmq
INCLUDEPATH += $$PWD/../include

SOURCES += main.cpp \
    pianowindow.cpp


HEADERS += \
    pianowindow.h
