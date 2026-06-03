#ifndef DATABASE_H
#define DATABASE_H

/**
 * @file database.h
 * @brief Заголовочный файл класса Database — Singleton для работы с БД.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 *
 * Реализует паттерн проектирования Singleton для SQLite базы данных.
 * Гарантирует единственное подключение к БД на всё время работы сервера.
 */

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

/**
 * @brief Класс для работы с базой данных пользователей.
 *
 * Реализует паттерн Singleton — объект создаётся только один раз.
 * Хранит таблицу пользователей с логинами, хешами паролей и ролями.
 *
 * Структура таблицы users:
 * - id: INTEGER PRIMARY KEY AUTOINCREMENT
 * - login: VARCHAR(50) UNIQUE
 * - password: VARCHAR(100) — SHA-256 хеш пароля
 * - role: VARCHAR(10) — "user" или "admin"
 */
class Database
{
public:
    /**
     * @brief Получить единственный экземпляр класса Database.
     *
     * При первом вызове создаёт объект и подключается к БД.
     * При последующих вызовах возвращает уже созданный объект.
     *
     * @return Указатель на единственный экземпляр Database
     */
    static Database* getInstance();

    /**
     * @brief Зарегистрировать нового пользователя в системе.
     *
     * Хеширует пароль через SHA-256 перед сохранением в БД.
     * Проверяет уникальность логина.
     *
     * @param login Логин нового пользователя
     * @param password Пароль в открытом виде (будет захеширован)
     * @param role Роль пользователя: "user" или "admin" (по умолчанию "user")
     * @return "ok: registered" при успехе или строку с ошибкой
     */
    QString registerUser(const QString &login,
                         const QString &password,
                         const QString &role = "user");

    /**
     * @brief Авторизовать пользователя по логину и паролю.
     *
     * Хеширует введённый пароль и сравнивает с хешем в БД.
     *
     * @param login Логин пользователя
     * @param password Пароль в открытом виде
     * @return Роль пользователя ("user"/"admin") или строку с ошибкой
     */
    QString loginUser(const QString &login, const QString &password);

    /**
     * @brief Получить список всех пользователей.
     *
     * Доступно только администратору.
     * Возвращает таблицу в формате "login|role\n..." 
     *
     * @return Строка с таблицей пользователей
     */
    QString getAllUsers();

    /**
     * @brief Удалить пользователя из системы.
     *
     * Доступно только администратору.
     * Нельзя удалить пользователя с логином "admin".
     *
     * @param login Логин удаляемого пользователя
     * @return "ok: deleted" при успехе или строку с ошибкой
     */
    QString deleteUser(const QString &login);

private:
    /**
     * @brief Приватный конструктор — запрещает создание объекта снаружи.
     *
     * Вызывает initDatabase() для подключения к БД.
     * Часть паттерна Singleton.
     */
    Database();

    Database(const Database&) = delete;            ///< Запрет копирования
    Database& operator=(const Database&) = delete; ///< Запрет присваивания

    /**
     * @brief Деструктор — закрывает соединение с БД.
     */
    ~Database();

    /**
     * @brief Инициализация базы данных.
     *
     * Создаёт файл users.db, таблицу users если её нет,
     * и администратора по умолчанию (admin/admin123).
     */
    void initDatabase();

    static Database* p_instance; ///< Единственный экземпляр (Singleton)
    QSqlDatabase db;              ///< Объект подключения к SQLite БД
};

#endif // DATABASE_H
