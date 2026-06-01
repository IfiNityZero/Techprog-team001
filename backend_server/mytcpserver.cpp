#include "mytcpserver.h"
#include "functionsforserver.h"
#include "database.h"
#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    // Инициализируем БД через Singleton
    Database::getInstance();

    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333))
        qDebug() << "server is not started";
    else
        qDebug() << "server is started";
}

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket *client = mTcpServer->nextPendingConnection();
    mClients.append(client);

    connect(client, &QTcpSocket::readyRead,
            this, &MyTcpServer::slotServerRead);
    connect(client, &QTcpSocket::disconnected,
            this, &MyTcpServer::slotClientDisconnected);

    client->write("Welcome! Please login or register.\r\n"
                  "Commands: register:LOGIN:PASS | login:LOGIN:PASS\r\n");

    qDebug() << "New client connected. Total:" << mClients.size();
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    while (client->bytesAvailable() > 0) {
        QByteArray array = client->readAll();
        QString request = QString::fromUtf8(array).trimmed();

        qDebug() << "Request from client:" << request;

        QString response = handleRequest(client, request);
        client->write((response + "\r\n").toUtf8());
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    qDebug() << "Client disconnected:" << mLoggedIn.value(client, "unknown");

    mClients.removeAll(client);
    mLoggedIn.remove(client);
    mRoles.remove(client);
    client->deleteLater();

    qDebug() << "Clients remaining:" << mClients.size();
}

QString MyTcpServer::handleRequest(QTcpSocket *client, const QString &request)
{
    QStringList parts = request.split(":");
    if (parts.isEmpty()) return "error";

    QString cmd = parts[0].toLower();

    // Регистрация
    if (cmd == "register") {
        if (parts.size() < 3) return "error: register:LOGIN:PASS";
        return Database::getInstance()->registerUser(parts[1], parts[2]);
    }

    // Авторизация
    if (cmd == "login") {
        if (parts.size() < 3) return "error: login:LOGIN:PASS";
        QString role = Database::getInstance()->loginUser(parts[1], parts[2]);
        if (role.startsWith("error")) return role;
        mLoggedIn[client] = parts[1];
        mRoles[client] = role;
        return "ok: logged in as " + role;
    }

    // Все остальные команды только для авторизованных
    if (!mLoggedIn.contains(client))
        return "error: please login first";

    QString role = mRoles.value(client, "user");

    // Команды только для админа
    if (cmd == "admin_users" || cmd == "admin_delete") {
        if (role != "admin") return "error: admin only";
        return handleAdminCommand(cmd, parts);
    }

    // Обычные команды
    return parsing(request);
}

QString MyTcpServer::handleAdminCommand(const QString &cmd,
                                         const QStringList &parts)
{
    if (cmd == "admin_users")
        return Database::getInstance()->getAllUsers();

    if (cmd == "admin_delete") {
        if (parts.size() < 2) return "error: admin_delete:LOGIN";
        return Database::getInstance()->deleteUser(parts[1]);
    }

    return "error: unknown admin command";
}
