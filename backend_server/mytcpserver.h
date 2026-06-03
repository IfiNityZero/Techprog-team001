#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

/**
 * @file mytcpserver.h
 * @brief Заголовочный файл TCP сервера с поддержкой нескольких клиентов.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 *
 * Сервер принимает подключения от нескольких клиентов одновременно,
 * обрабатывает авторизацию и выполняет команды в зависимости от роли.
 */

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>
#include <QMap>

/**
 * @brief TCP сервер с авторизацией, ролями и поддержкой нескольких клиентов.
 *
 * Наследуется от QObject для поддержки сигналов и слотов Qt.
 * Использует асинхронную обработку через события — один поток
 * обслуживает всех клиентов через Qt Event Loop.
 *
 * Поддерживаемые команды:
 * - register:LOGIN:PASS — регистрация
 * - login:LOGIN:PASS — авторизация
 * - vigenere_encrypt/decrypt:TEXT:KEY — шифр Виженера
 * - sha384:TEXT — хеширование
 * - chord:A:B:EPS — метод хорд
 * - stego_encode/decode:PATH — стеганография
 * - admin_users — список пользователей (только admin)
 * - admin_delete:LOGIN — удаление пользователя (только admin)
 */
class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор сервера.
     *
     * Инициализирует БД через Singleton, создаёт QTcpServer
     * и начинает слушать порт 33333.
     *
     * @param parent Родительский объект Qt (по умолчанию nullptr)
     */
    explicit MyTcpServer(QObject *parent = nullptr);

    /**
     * @brief Деструктор — закрывает серверный сокет.
     */
    ~MyTcpServer();

public slots:
    /**
     * @brief Слот обработки нового подключения клиента.
     *
     * Вызывается автоматически при новом подключении.
     * Добавляет сокет в список клиентов, отправляет приветствие.
     */
    void slotNewConnection();

    /**
     * @brief Слот обработки отключения клиента.
     *
     * Вызывается при разрыве соединения.
     * Удаляет клиента из всех структур данных.
     */
    void slotClientDisconnected();

    /**
     * @brief Слот чтения данных от клиента.
     *
     * Вызывается когда клиент прислал данные.
     * Читает запрос, передаёт в handleRequest() и отправляет ответ.
     */
    void slotServerRead();

private:
    /**
     * @brief Обработка запроса с учётом авторизации и роли клиента.
     *
     * Проверяет: авторизован ли клиент, какая у него роль,
     * и передаёт команду в соответствующий обработчик.
     *
     * @param client Указатель на сокет клиента
     * @param request Строка запроса от клиента
     * @return Строка с результатом выполнения команды
     */
    QString handleRequest(QTcpSocket *client, const QString &request);

    /**
     * @brief Обработка команд администратора.
     *
     * Выполняет admin_users и admin_delete через Database Singleton.
     *
     * @param cmd Команда ("admin_users" или "admin_delete")
     * @param parts Список аргументов команды
     * @return Строка с результатом
     */
    QString handleAdminCommand(const QString &cmd, const QStringList &parts);

    QTcpServer *mTcpServer; ///< Серверный сокет — принимает новые подключения

    QList<QTcpSocket*> mClients;           ///< Список всех подключённых клиентов
    QMap<QTcpSocket*, QString> mLoggedIn;  ///< Авторизованные клиенты: сокет → логин
    QMap<QTcpSocket*, QString> mRoles;     ///< Роли клиентов: сокет → "user"/"admin"
};

#endif // MYTCPSERVER_H
