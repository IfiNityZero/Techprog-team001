#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

/**
 * @brief Класс Database — Singleton для работы с БД SQLite.
 *
 * Обеспечивает единственное подключение к базе данных
 * и методы регистрации/авторизации пользователей.
 */
class Database
{
public:
    /**
     * @brief Получить единственный экземпляр Database.
     * @return указатель на экземпляр Database
     */
    static Database* getInstance();

    /**
     * @brief Зарегистрировать нового пользователя.
     * @param login логин
     * @param password пароль
     * @param role роль: "user" или "admin"
     * @return "ok" или сообщение об ошибке
     */
    QString registerUser(const QString &login,
                         const QString &password,
                         const QString &role = "user");

    /**
     * @brief Авторизовать пользователя.
     * @param login логин
     * @param password пароль
     * @return роль пользователя ("user"/"admin") или "error"
     */
    QString loginUser(const QString &login, const QString &password);

    /**
     * @brief Получить список всех пользователей (только для админа).
     * @return строка с таблицей пользователей
     */
    QString getAllUsers();

    /**
     * @brief Удалить пользователя (только для админа).
     * @param login логин удаляемого пользователя
     * @return "ok" или сообщение об ошибке
     */
    QString deleteUser(const QString &login);

private:
    Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    ~Database();

    void initDatabase();

    static Database* p_instance;
    QSqlDatabase db;
};

#endif // DATABASE_H
