//includes
#include "pianowindow.h"
#include <Windows.h>
#include <iostream>
#include <zmq.hpp>
#include <string>
#include <sstream>

#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <iostream>
#include <QVBoxLayout>
#include <QFrame>
#include <QDebug>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>

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

zmq::context_t context(1);
zmq::socket_t publisher(context,  local ? ZMQ_PUB : ZMQ_PUSH);
zmq::socket_t receiver(context, ZMQ_SUB);

int vel = 100;
int pan = 0;
std::string clientID = "jeff";

void init()
{
    if (local)
    {
        publisher.connect(ladress2); // connect to service's SUB
        receiver.connect(ladress1);  // connect to service's PUB
        std::cout << "Connected locally to " << ladress1 << " and " << ladress2 << std::endl;
    } else
    {
        publisher.connect(adress1); // connect to service's SUB
        receiver.connect(adress2);  // connect to service's PUB
        std::cout << "Connected to Benternet at " << adress1 << " and " << adress2 << std::endl;
    }

    receiver.setsockopt(ZMQ_SUBSCRIBE, "readnum!>", 9);
    receiver.setsockopt(ZMQ_RCVTIMEO, 3000);
    std::cout << "Subscribed to 'readnum!>'" << std::endl;

    sleep(1000); // give time for connections to fully register
}

void send_note(int state, int note, int velocity)
{
    std::ostringstream ss;
    ss << "pynqsynth@note.play?>=" << clientID << "="<< state << " " << note << " " << velocity;
    std::string request = ss.str();

    zmq::message_t msg(request.begin(), request.end());
    publisher.send(msg);
    std::cout << "Sent request: " << request << std::endl;
    sleep(1);
}

void switch_instrument(int instrument)
{
    std::ostringstream ss;
    ss << "pynqsynth@instrument.change?>=" << clientID << "=" << instrument;
    std::string request = ss.str();

    zmq::message_t msg(request.begin(), request.end());
    publisher.send(msg);
    std::cout << "Sent request: " << request << std::endl;
    sleep(1);
}

void send_effect(int effect)
{
    std::ostringstream ss;
    ss << "pynqsynth@effect.change?>=" << clientID << "="  << effect;
    std::string request = ss.str();

    zmq::message_t msg(request.begin(), request.end());
    publisher.send(msg);
    std::cout << "Sent request: " << request << std::endl;
    sleep(1);
}

void set_pan(int pan)
{
    std::ostringstream ss;
    ss << "pynqsynth@set.pan?>=" << clientID << "=" << pan;
    std::string request = ss.str();

    zmq::message_t msg(request.begin(), request.end());
    publisher.send(msg);
    std::cout << "Sent request: " << request << std::endl;
    sleep(1);
}


void recieve()
{
    zmq::message_t reply;
    if (receiver.recv(&reply)) {
        std::string response(static_cast<char*>(reply.data()), reply.size());
        std::cout << "Received response: " << response << std::endl;
    } else {
        std::cout << "No response received (timeout)." << std::endl;
    }
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    init();

    app.setAttribute(Qt::AA_DisableHighDpiScaling); // if needed
    app.setKeyboardInputInterval(0);

    PianoWindow window;
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);


    QHBoxLayout *clientIDLayout = new QHBoxLayout();
    QLabel *clientIDLabel = new QLabel("Client ID:");
    QLineEdit *clientIDInput = new QLineEdit(QString::fromStdString(clientID));
    clientIDLayout->addWidget(clientIDLabel);
    clientIDLayout->addWidget(clientIDInput);
    mainLayout->addLayout(clientIDLayout);

    QObject::connect(clientIDInput, &QLineEdit::textChanged, [](const QString &text) {
        clientID = text.toStdString();
        qDebug() << "Client ID set to:" << text;
    });


    //custom intrument
    QGroupBox *customGroup = new QGroupBox("Custom Instrument Settings");
    QVBoxLayout *customLayout = new QVBoxLayout(customGroup);

    QComboBox *waveformBox = new QComboBox();
    waveformBox->addItems({"Sine", "Square", "Triangle", "Saw"});
    customLayout->addWidget(new QLabel("Waveform:"));
    customLayout->addWidget(waveformBox);

    QSlider *attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setRange(0, 100); attackSlider->setValue(10);
    QSlider *decaySlider = new QSlider(Qt::Horizontal);
    decaySlider->setRange(0, 100); decaySlider->setValue(20);
    QSlider *sustainSlider = new QSlider(Qt::Horizontal);
    sustainSlider->setRange(0, 100); sustainSlider->setValue(70);
    QSlider *releaseSlider = new QSlider(Qt::Horizontal);
    releaseSlider->setRange(0, 100); releaseSlider->setValue(30);

    customLayout->addWidget(new QLabel("Attack")); customLayout->addWidget(attackSlider);
    customLayout->addWidget(new QLabel("Decay")); customLayout->addWidget(decaySlider);
    customLayout->addWidget(new QLabel("Sustain")); customLayout->addWidget(sustainSlider);
    customLayout->addWidget(new QLabel("Release")); customLayout->addWidget(releaseSlider);

    customGroup->setVisible(false); // hidden unless "Custom" is selected
    mainLayout->addWidget(customGroup);

    auto send_custom_settings = [=]() {
        int waveform = waveformBox->currentIndex();
        int attack = attackSlider->value();
        int decay = decaySlider->value();
        int sustain = sustainSlider->value();
        int release = releaseSlider->value();

        std::ostringstream ss;
        ss << "pynqsynth@custom.instrument?>=" << clientID << "="
           << waveform << " "
           << attack << " "
           << decay << " "
           << sustain << " "
           << release;

        std::string msg = ss.str();
        zmq::message_t zmqMsg(msg.begin(), msg.end());
        publisher.send(zmqMsg);
        std::cout << "Sent custom settings: " << msg << std::endl;
    };

    QObject::connect(waveformBox, QOverload<int>::of(&QComboBox::currentIndexChanged), send_custom_settings);
    QObject::connect(attackSlider, &QSlider::valueChanged, send_custom_settings);
    QObject::connect(decaySlider, &QSlider::valueChanged, send_custom_settings);
    QObject::connect(sustainSlider, &QSlider::valueChanged, send_custom_settings);
    QObject::connect(releaseSlider, &QSlider::valueChanged, send_custom_settings);

    // Dropdown for instruments
    QComboBox *instrumentBox = new QComboBox();
    instrumentBox->addItems({"Piano", "Synth", "Organ", "Guitar","Custom"});
    mainLayout->addWidget(instrumentBox);
    QObject::connect(instrumentBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [=](int index) {
                         QString instrument =instrumentBox->itemText(index);
                         std::cout  << "Instrument changed to:" << instrument.toStdString() <<std::endl;
                         switch_instrument(instrumentBox->currentIndex());
                         customGroup->setVisible(index == 4);
                     });



    // Volume slider
    QHBoxLayout *volumeLayout = new QHBoxLayout();
    QLabel *volumeLabel = new QLabel("Volume:");
    QSlider *volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 127);
    volumeSlider->setValue(vel);
    volumeLayout->addWidget(volumeLabel);
    volumeLayout->addWidget(volumeSlider);
    mainLayout->addLayout(volumeLayout);

    QObject::connect(volumeSlider, &QSlider::valueChanged, [](int value) {
        vel = value;
        qDebug() << "Volume set to:" << vel;
    });

    // pan slider
    QHBoxLayout *panLayout = new QHBoxLayout();
    QLabel *panLabel = new QLabel("Pan:");
    QSlider *panSlider = new QSlider(Qt::Horizontal);
    panSlider->setRange(-100, 100);
    panSlider->setValue(pan);
    panLayout->addWidget(panLabel);
    panLayout->addWidget(panSlider);
    mainLayout->addLayout(panLayout);

    QObject::connect(panSlider, &QSlider::valueChanged, [](int value) {
        pan = value;
        set_pan(pan);
        qDebug() << "Pan set to:" << pan;
    });

    QWidget *piano = new QWidget();
    piano->setFixedSize(7 * 40, 160);
    piano->setStyleSheet("background: white;");

    const QString whiteLabels[] = {"C", "D", "E", "F", "G", "A", "B"};
    const int whiteNotes[] = {60, 62, 64, 65, 67, 69, 71};
    const int blackNotes[] = {61, 63, 66, 68, 70};
    const int blackOffsets[] = {30, 70, 150, 190, 230};

    for (int i = 0; i < 7; ++i) {
        QPushButton *w = new QPushButton(whiteLabels[i], piano);
        w->setGeometry(i * 40, 0, 40, 160);
        QObject::connect(w, &QPushButton::pressed, [=]() {
            qDebug() << "White note ON:" << whiteNotes[i];
            send_note(1, whiteNotes[i], vel);
        });
        QObject::connect(w, &QPushButton::released, [=]() {
            qDebug() << "White note OFF:" << whiteNotes[i];
            send_note(0, whiteNotes[i], vel);
        });
    }

    for (int i = 0; i < 5; ++i) {
        QPushButton *b = new QPushButton("", piano);
        b->setStyleSheet("background-color: black;");
        b->setGeometry(blackOffsets[i], 0, 30, 100);
        b->raise();
        QObject::connect(b, &QPushButton::pressed, [=]() {
            qDebug() << "Black note ON:" << blackNotes[i];
            send_note(1, blackNotes[i], vel);
        });
        QObject::connect(b, &QPushButton::released, [=]() {
            qDebug() << "Black note OFF:" << blackNotes[i];
            send_note(0, blackNotes[i], vel);
        });
    }

    mainLayout->addWidget(piano);

    // Effects dropdown
    QComboBox *effectsBox = new QComboBox();
    effectsBox->addItems({"None",  "Echo", "Tremolo", "Distortion", "Low pass", "Bitcrush", "Reverb"});
    mainLayout->addWidget(new QLabel("Effects:"));
    mainLayout->addWidget(effectsBox);

    QObject::connect(effectsBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [=](int index) {
                         QString effect = effectsBox->itemText(index);
                         std::cout << "Effect changed to: " << effect.toStdString() << std::endl;
                         send_effect(index);
                     });

    window.setWindowTitle("Piano Keys");
    window.show();
    return app.exec();

}
