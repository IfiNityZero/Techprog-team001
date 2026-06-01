#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>
#include <QMap>

/**
 * @brief Класс MyTcpServer — TCP сервер с поддержкой
 * нескольких клиентов, авторизации и ролей.
 */
class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();

private:
    QString handleRequest(QTcpSocket *client, const QString &request);
    QString handleAdminCommand(const QString &cmd,
                               const QStringList &parts);

    QTcpServer *mTcpServer;

    // Список подключённых клиентов
    QList<QTcpSocket*> mClients;

    // Авторизованные клиенты: сокет → логин
    QMap<QTcpSocket*, QString> mLoggedIn;

    // Роли клиентов: сокет → роль ("user"/"admin")
    QMap<QTcpSocket*, QString> mRoles;
};

#endif // MYTCPSERVER_H
