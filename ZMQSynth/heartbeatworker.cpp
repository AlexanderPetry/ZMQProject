#include "heartbeatworker.h"
#include <thread>

HeartbeatWorker::HeartbeatWorker(zmq::context_t& ctx, bool localMode, QObject* parent)
    : QObject(parent), context(ctx), local(localMode), running(true)
{}

HeartbeatWorker::~HeartbeatWorker() {
    stop();
}

void HeartbeatWorker::process() {
    zmq::socket_t sender(context, local ? ZMQ_PUB : ZMQ_PUSH);
    if (local) sender.bind("tcp://127.0.0.1:24041");
    else sender.connect("tcp://benternet.pxl-ea-ict.be:24041");

    while (running) {
        std::string msg = "pynqsynth@status.reply!>alive";
        zmq::message_t message(msg.begin(), msg.end());
        sender.send(message);
        QThread::sleep(5);
    }
    emit finished();
}

void HeartbeatWorker::stop() {
    running = false;
}
