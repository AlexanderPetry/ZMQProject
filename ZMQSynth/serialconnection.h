#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <QByteArray>
#include <QtTypes>
#include <QSerialPort>
#include <QTimer>

static constexpr std::chrono::seconds kWriteTimeout = std::chrono::seconds{5};

class SerialConnection : public QObject
{
    Q_OBJECT
public:
    QSerialPort *sp;
    QTimer *tm;

    qint64 bytesToWrite = 0;

    SerialConnection();
    void openSerialPort(QString portName, int baudRate);
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void handleBytesWritten(qint64 bytes);
    QByteArray readData();

public slots:
    void handleReadyRead();


};


#endif // SERIALCONNECTION_H
