#include "mytcpserver.h"
#include "functionsforserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QString>

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();
    mTcpSocket->write("Hello! I am TaMP server. Send your command.\r\n");
    connect(mTcpSocket, &QTcpSocket::readyRead,
            this, &MyTcpServer::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::disconnected,
            this, &MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotServerRead()
{
    while (mTcpSocket->bytesAvailable() > 0)
    {
        QByteArray array = mTcpSocket->readAll();
        QString request = QString::fromUtf8(array).trimmed();

        qDebug() << "Получена команда:" << request;

        // Передаём команду в parsing() и отправляем результат
        QString response = parsing(request);

        qDebug() << "Ответ:" << response;

        mTcpSocket->write((response + "\r\n").toUtf8());
    }
}

void MyTcpServer::slotClientDisconnected()
{
    mTcpSocket->close();
}
