#include "synth.h"

synth::synth() {}

SerialConnection *sc;

bool local = 0;

std::string adress1 = "tcp://benternet.pxl-ea-ict.be:24041";
std::string adress2 = "tcp://benternet.pxl-ea-ict.be:24042";
std::string ladress1 = "tcp://127.0.0.1:24041";
std::string ladress2 = "tcp://127.0.0.1:24042";

zmq::socket_t* logSocket = nullptr;
