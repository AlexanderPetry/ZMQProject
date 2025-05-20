#include "serialconnection.h"
#include <iostream>


SerialConnection::SerialConnection()
{

}

void SerialConnection::openSerialPort(QString portName, int baudRate)
{
    sp = new QSerialPort();
    sp->setPortName(portName);
    sp->setBaudRate(baudRate);
    sp->setDataBits(QSerialPort::Data8);
    sp->setParity(QSerialPort::NoParity);
    sp->setFlowControl(QSerialPort::SoftwareControl);
    if (sp->open(QIODevice::ReadWrite)) {
        std::cout << "Serial port opened successfully:" << portName.toStdString() << std::endl;;
    } else {
        std::cout << "Failed to open serial port:" << portName.toStdString() << "-" << sp->errorString().toStdString() << std::endl;;
    }

    tm = new QTimer(this);
    tm->setSingleShot(true);

    connect(sp, &QSerialPort::readyRead, this, &SerialConnection::handleReadyRead);
}

void SerialConnection::closeSerialPort()
{
    if(sp->isOpen())
    {
        sp->close();
        std::cout << "Serial port closed.";
    }
}

void SerialConnection::writeData(const QByteArray &data)
{
    const qint64 written = sp->write(data);
    if(written == data.size())
    {
        std::cout << "Data written successfully:" << data.toHex().toStdString() << std::endl;
        bytesToWrite += written;
        tm->start(kWriteTimeout);
    }
    else
    {
        std::cout << "Partial or failed write. Written:" << written << "Expected:" << data.size();
    }
}

void SerialConnection::handleBytesWritten(qint64 bytes)
{
    bytesToWrite -= bytes;
    if(bytesToWrite == 0)
    {
        tm->stop();
        std::cout << "All bytes written.";
    }
}

QByteArray SerialConnection::readData()
{
    QByteArray data = sp->readAll();
    std::cout << "Data read:" << data.toHex().toStdString() << std::endl;
    return data;
}

void SerialConnection::handleReadyRead() {
    QByteArray data = sp->readAll();
    if (!data.isEmpty()) {
        std::string text = data.toStdString();
        std::cout << "Serial in (text): " << text << std::flush;
    }
}
