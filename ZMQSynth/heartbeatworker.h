#ifndef HEARTBEATWORKER_H
#define HEARTBEATWORKER_H

#include <QObject>
#include <zmq.hpp>
#include <QThread>

class HeartbeatWorker : public QObject {
    Q_OBJECT

public:
    explicit HeartbeatWorker(zmq::context_t& ctx, bool localMode, QObject* parent = nullptr);
    ~HeartbeatWorker();

public slots:
    void process();
    void stop();

signals:
    void finished();

private:
    zmq::context_t& context;
    bool local;
    bool running = true;
};

#endif // HEARTBEATWORKER_H
