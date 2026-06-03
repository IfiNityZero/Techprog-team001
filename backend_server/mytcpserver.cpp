/**
 * @file mytcpserver.cpp
 * @brief Реализация TCP сервера с авторизацией и поддержкой нескольких клиентов.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 */

#include "mytcpserver.h"
#include "functionsforserver.h"
#include "database.h"
#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    Database::getInstance(); // инициализируем БД через Singleton при старте сервера

    mTcpServer = new QTcpServer(this); // создаём серверный сокет

    // подключаем сигнал newConnection к нашему слоту slotNewConnection
    // сигнал срабатывает автоматически при новом подключении клиента
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    // начинаем слушать на всех интерфейсах (0.0.0.0), порт 33333
    if (!mTcpServer->listen(QHostAddress::Any, 33333))
        qDebug() << "server is not started";
    else
        qDebug() << "server is started";
}

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close(); // закрываем сервер — перестаём принимать новые подключения
}

void MyTcpServer::slotNewConnection()
{
    // получаем сокет нового клиента из очереди ожидающих подключений
    QTcpSocket *client = mTcpServer->nextPendingConnection();

    mClients.append(client); // добавляем в список всех клиентов

    // подключаем сигналы сокета к нашим слотам
    connect(client, &QTcpSocket::readyRead,    // данные готовы к чтению
            this, &MyTcpServer::slotServerRead);
    connect(client, &QTcpSocket::disconnected, // клиент отключился
            this, &MyTcpServer::slotClientDisconnected);

    // отправляем приветственное сообщение новому клиенту
    client->write("Welcome! Please login or register.\r\n"
                  "Commands: register:LOGIN:PASS | login:LOGIN:PASS\r\n");

    qDebug() << "New client connected. Total:" << mClients.size();
}

void MyTcpServer::slotServerRead()
{
    // sender() возвращает объект который испустил сигнал
    // так мы узнаём от какого именно клиента пришли данные
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return; // защита от некорректного вызова

    while (client->bytesAvailable() > 0) // читаем пока есть данные
    {
        QByteArray array = client->readAll(); // читаем все доступные байты

        // конвертируем байты в строку и убираем лишние пробелы/переносы
        QString request = QString::fromUtf8(array).trimmed();

        qDebug() << "Request from client:" << request;

        QString response = handleRequest(client, request); // обрабатываем запрос

        client->write((response + "\r\n").toUtf8()); // отправляем ответ
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    qDebug() << "Client disconnected:" << mLoggedIn.value(client, "unknown");

    mClients.removeAll(client);  // удаляем из списка клиентов
    mLoggedIn.remove(client);    // удаляем из авторизованных
    mRoles.remove(client);       // удаляем роль
    client->deleteLater();       // безопасно удаляем объект сокета

    qDebug() << "Clients remaining:" << mClients.size();
}

QString MyTcpServer::handleRequest(QTcpSocket *client, const QString &request)
{
    QStringList parts = request.split(":"); // разбиваем запрос по ':'
    if (parts.isEmpty()) return "error";

    QString cmd = parts[0].toLower(); // команда в нижнем регистре

    // --- Регистрация (доступна всем без авторизации) ---
    if (cmd == "register") {
        if (parts.size() < 3) return "error: register:LOGIN:PASS";
        return Database::getInstance()->registerUser(parts[1], parts[2]);
    }

    // --- Авторизация (доступна всем без авторизации) ---
    if (cmd == "login") {
        if (parts.size() < 3) return "error: login:LOGIN:PASS";

        QString role = Database::getInstance()->loginUser(parts[1], parts[2]);

        if (role.startsWith("error")) return role; // неверные данные

        mLoggedIn[client] = parts[1]; // запоминаем что клиент авторизован
        mRoles[client]    = role;     // запоминаем роль клиента

        return "ok: logged in as " + role;
    }

    // --- Все остальные команды требуют авторизации ---
    if (!mLoggedIn.contains(client)) // клиент не в списке авторизованных
        return "error: please login first";

    QString role = mRoles.value(client, "user"); // получаем роль клиента

    // --- Команды только для администратора ---
    if (cmd == "admin_users" || cmd == "admin_delete") {
        if (role != "admin") return "error: admin only"; // проверяем роль
        return handleAdminCommand(cmd, parts);
    }

    // --- Обычные команды (доступны всем авторизованным) ---
    return parsing(request); // передаём в парсер алгоритмов
}

QString MyTcpServer::handleAdminCommand(const QString &cmd,
                                         const QStringList &parts)
{
    if (cmd == "admin_users")
        return Database::getInstance()->getAllUsers(); // список всех пользователей

    if (cmd == "admin_delete") {
        if (parts.size() < 2) return "error: admin_delete:LOGIN";
        return Database::getInstance()->deleteUser(parts[1]); // удаляем пользователя
    }

    return "error: unknown admin command";
}
