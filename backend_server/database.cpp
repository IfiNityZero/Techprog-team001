/**
 * @file database.cpp
 * @brief Реализация класса Database — Singleton для работы с SQLite БД.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 */

#include "database.h"
#include <QCryptographicHash>
#include <QDebug>

// Инициализация статического указателя — изначально объекта нет
Database* Database::p_instance = nullptr;

Database* Database::getInstance()
{
    if (!p_instance)               // если объект ещё не создан
        p_instance = new Database(); // создаём его (единственный раз)
    return p_instance;             // возвращаем указатель на объект
}

Database::Database()
{
    initDatabase(); // при создании сразу подключаемся к БД
}

Database::~Database()
{
    if (db.isOpen()) // если соединение открыто — закрываем
        db.close();
}

void Database::initDatabase()
{
    // добавляем SQLite драйвер — встроен в Qt, не требует установки
    db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName("users.db"); // имя файла БД (создастся автоматически)

    if (!db.open()) // пытаемся открыть / создать файл БД
    {
        qDebug() << "DB error:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);

    // создаём таблицу пользователей если она ещё не существует
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT," // уникальный ID
               "login VARCHAR(50) NOT NULL UNIQUE,"    // логин (уникальный)
               "password VARCHAR(100) NOT NULL,"       // SHA-256 хеш пароля
               "role VARCHAR(10) NOT NULL DEFAULT 'user'" // роль пользователя
               ")");

    // проверяем есть ли хоть один пользователь в таблице
    query.exec("SELECT COUNT(*) FROM users");

    if (query.next() && query.value(0).toInt() == 0) // таблица пустая
    {
        // создаём пароль администратора по умолчанию: SHA-256("admin123")
        QString adminPass = QString(QCryptographicHash::hash(
            "admin123",
            QCryptographicHash::Sha256
        ).toHex());

        // вставляем администратора в таблицу
        query.prepare("INSERT INTO users (login, password, role) "
                      "VALUES (:l, :p, :r)");
        query.bindValue(":l", "admin");     // логин
        query.bindValue(":p", adminPass);   // хеш пароля
        query.bindValue(":r", "admin");     // роль
        query.exec();

        qDebug() << "Default admin created: login=admin, password=admin123";
    }

    qDebug() << "Database initialized successfully";
}

QString Database::registerUser(const QString &login,
                                const QString &password,
                                const QString &role)
{
    if (!db.isOpen()) return "error: database not connected";
    if (login.isEmpty() || password.isEmpty()) return "error: empty fields";

    // хешируем пароль перед сохранением — никогда не храним пароль в открытом виде
    QString hashedPass = QString(QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
    ).toHex());

    QSqlQuery query(db);

    // подготавливаем запрос с параметрами (защита от SQL-инъекций)
    query.prepare("INSERT INTO users (login, password, role) "
                  "VALUES (:l, :p, :r)");
    query.bindValue(":l", login);      // подставляем логин
    query.bindValue(":p", hashedPass); // подставляем хеш пароля
    query.bindValue(":r", role);       // подставляем роль

    if (!query.exec()) // если запрос не выполнился (например дубликат логина)
        return "error: user already exists";

    return "ok: registered";
}

QString Database::loginUser(const QString &login, const QString &password)
{
    if (!db.isOpen()) return "error";

    // хешируем введённый пароль для сравнения с хешем в БД
    QString hashedPass = QString(QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
    ).toHex());

    QSqlQuery query(db);

    // ищем пользователя с таким логином и хешем пароля
    query.prepare("SELECT role FROM users WHERE login=:l AND password=:p");
    query.bindValue(":l", login);
    query.bindValue(":p", hashedPass);

    if (!query.exec() || !query.next()) // пользователь не найден
        return "error: wrong login or password";

    return query.value(0).toString(); // возвращаем роль: "user" или "admin"
}

QString Database::getAllUsers()
{
    if (!db.isOpen()) return "error: database not connected";

    QSqlQuery query(db);
    query.exec("SELECT login, role FROM users"); // получаем всех пользователей

    QString result = "login|role\n"; // заголовок таблицы

    while (query.next()) // перебираем все строки результата
    {
        result += query.value(0).toString() + "|" + // логин
                  query.value(1).toString() + "\n";  // роль
    }

    return result.trimmed(); // убираем лишний перенос строки в конце
}

QString Database::deleteUser(const QString &login)
{
    if (!db.isOpen()) return "error: database not connected";

    if (login == "admin") // защита от удаления главного администратора
        return "error: cannot delete admin";

    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE login=:l"); // удаляем по логину
    query.bindValue(":l", login);

    if (!query.exec() || query.numRowsAffected() == 0) // пользователь не найден
        return "error: user not found";

    return "ok: deleted";
}
