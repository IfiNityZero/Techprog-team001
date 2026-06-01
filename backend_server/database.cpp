#include "database.h"
#include <QCryptographicHash>
#include <QDebug>

// Инициализация статического указателя
Database* Database::p_instance = nullptr;

Database* Database::getInstance()
{
    if (!p_instance)
        p_instance = new Database();
    return p_instance;
}

Database::Database()
{
    initDatabase();
}

Database::~Database()
{
    if (db.isOpen())
        db.close();
}

void Database::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("users.db");

    if (!db.open()) {
        qDebug() << "DB error:" << db.lastError().text();
        return;
    }

    // Создаём таблицу пользователей если её нет
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "login VARCHAR(50) NOT NULL UNIQUE,"
               "password VARCHAR(100) NOT NULL,"
               "role VARCHAR(10) NOT NULL DEFAULT 'user'"
               ")");

    // Создаём admin по умолчанию если таблица пустая
    query.exec("SELECT COUNT(*) FROM users");
    if (query.next() && query.value(0).toInt() == 0) {
        QString adminPass = QString(QCryptographicHash::hash(
            "admin123", QCryptographicHash::Sha256).toHex());
        query.prepare("INSERT INTO users (login, password, role) "
                      "VALUES (:l, :p, :r)");
        query.bindValue(":l", "admin");
        query.bindValue(":p", adminPass);
        query.bindValue(":r", "admin");
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

    // Хешируем пароль
    QString hashedPass = QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (login, password, role) "
                  "VALUES (:l, :p, :r)");
    query.bindValue(":l", login);
    query.bindValue(":p", hashedPass);
    query.bindValue(":r", role);

    if (!query.exec())
        return "error: user already exists";

    return "ok: registered";
}

QString Database::loginUser(const QString &login, const QString &password)
{
    if (!db.isOpen()) return "error";

    QString hashedPass = QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());

    QSqlQuery query(db);
    query.prepare("SELECT role FROM users WHERE login=:l AND password=:p");
    query.bindValue(":l", login);
    query.bindValue(":p", hashedPass);

    if (!query.exec() || !query.next())
        return "error: wrong login or password";

    return query.value(0).toString(); // "user" или "admin"
}

QString Database::getAllUsers()
{
    if (!db.isOpen()) return "error: database not connected";

    QSqlQuery query(db);
    query.exec("SELECT login, role FROM users");

    QString result = "login|role\n";
    while (query.next())
        result += query.value(0).toString() + "|" +
                  query.value(1).toString() + "\n";

    return result.trimmed();
}

QString Database::deleteUser(const QString &login)
{
    if (!db.isOpen()) return "error: database not connected";
    if (login == "admin") return "error: cannot delete admin";

    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE login=:l");
    query.bindValue(":l", login);

    if (!query.exec() || query.numRowsAffected() == 0)
        return "error: user not found";

    return "ok: deleted";
}
