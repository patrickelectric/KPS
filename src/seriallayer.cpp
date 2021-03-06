#include <QDebug>

#include "seriallayer.h"

QByteArray SerialLayer::_return = QByteArray("\r");
QByteArray SerialLayer::_newLine = QByteArray("\n");
QByteArray SerialLayer::_newLineReturn = QByteArray("\n\r");

SerialLayer::SerialLayer(QString port, uint baud, QWidget *parent) :
    serial(new QSerialPort()),
    _serialOpened(false),
    existTimer(new QTimer())
{
    serial->setPortName(port);
    serial->setBaudRate(baud);
    _serialOpened = serial->open(QIODevice::ReadWrite);

    existTimer->setInterval(1);
    connect(existTimer, &QTimer::timeout, this, &SerialLayer::emitExistCommand);

    connect(serial, &QSerialPort::readyRead, this, &SerialLayer::readData);
};

bool SerialLayer::opened()
{
    return _serialOpened;
}

void SerialLayer::readData()
{
    _rawData.append(serial->readAll());

    /*
     * Check if \r exist and remove
     * Both \n\r and \n are used in string's end and some protocols
     */
    if (_rawData.contains(_return))
    {
        _rawData = _rawData.replace(_return, QByteArray());
    }

    QByteArray tempArray;
    QList<QByteArray> tempList = _rawData.split(_newLine.at(0));
    for (auto i = tempList.begin(); i != tempList.end(); ++i)
    {
        // Get finished line to _byteCommands
        if (i < tempList.end()-1)
        {
            _rByteCommands.append(*i);
            emit(receivedCommand(*i));

            if(!existTimer->isActive())
            {
                existTimer->start();
            }
        }
        else
        {
            _rawData.clear();
            _rawData.append(*i);
        }
    }
}

void SerialLayer::pushCommand(QByteArray comm, QByteArray term)
{
    comm.append(term);
    serial->write(comm);
    emit(pushedCommand(comm));

}

void SerialLayer::pushCommand(QByteArray comm)
{
    pushCommand(comm, _newLineReturn);
}

void SerialLayer::add(QByteArray comm, QByteArray term)
{
    comm.append(term);
    _sByteCommands.append(comm);
}

void SerialLayer::add(QByteArray comm)
{
    add(comm, _newLineReturn);
}

void SerialLayer::push()
{
    foreach(const auto& comm, _sByteCommands)
    {
        serial->write(comm);
        emit(pushedCommand(comm));
    }
    _sByteCommands.clear();
}

void SerialLayer::emitExistCommand()
{
    if(commandAvailable())
    {
        emit(existCommand(peekCommand()));
    }
    else
    {
        existTimer->stop();
    }
}

QByteArray SerialLayer::peekCommand()
{
    return commandAvailable() ? _rByteCommands.first() : QByteArray();
}

QByteArray SerialLayer::popCommand()
{
    return commandAvailable() ? _rByteCommands.takeFirst() : QByteArray();
}

bool SerialLayer::commandAvailable()
{
    return !_rByteCommands.isEmpty();
}

SerialLayer::~SerialLayer()
{
    delete serial;
}
void SerialLayer::closeConnection()
{
    if(_serialOpened)
    {
        serial->close();
    }
}
