#include <QCoreApplication>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // создаем базу данных типа QSQLite
    // QSQLite - встроенный драйвер в QT
    //  QPSQL
    QSqlDatabase db =
            QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Test.db");

    if(!db.open())
        qDebug()<<db.lastError().text();

    // добавить запись путем SQL запроса
    // создать запрос к БД
   /*
    создание таблицы

    Название: User
    Поля: login, password, status - хранятся значениия типа char[20]
                                         (массив символов длины 20)

    CREATE TABLE User(login VARCHAR(20) NOT NULL,
                    password VARCHAR(20) NOT NULL,
                      status VARCHAR(20) NOT NULL)

    Зачем нужны типы данных?
        Пример: в поле age типа INTEGER в разных записях БД (строках таблицы) записано
                "123"
                34
                "a"
        Вопрос: как сравнивать? преобразуем к заданному типу INTEGER:
                    "123" -> 123
                    34 -> 34
                    "a" -> 65 (код символа)

    Вставить запись в таблицу

    Название таблицы: User
    login: "admin"
    password: "123"

    INSERT INTO User(login, password) VALUES ("admin", "123")
*/
    QSqlQuery query(db);
    //создали таблицу
    query.exec("CREATE TABLE User("
               "login VARCHAR(20) NOT NULL, "
               "password VARCHAR(20) NOT NULL"
               ")");

    //добавить запись в таблицу
    /*query.prepare("INSERT INTO User(login, password) VALUES (\"admin\", \"123\")");


   INSERT INTO User(login, password)
                  VALUES (:login, :password)
*/
    query.prepare("INSERT INTO User(login, password) "
                      "VALUES (:login, :password)");

    query.bindValue(":password","123");
    query.bindValue(":login","admin");

    query.exec();// выполнить запрос


    query.prepare("INSERT INTO User(login, password) "
                      "VALUES (:login, :password)");

    query.bindValue(":password","123");
    query.bindValue(":login","user");

    query.exec();// выполнить запрос

    /*
    Выбрать все записи БД
    SELECT * FROM User

    Выбрать все записи БД с условием
    SELECT * FROM User WHERE .......
   */
    query.exec("SELECT * FROM User");


    // вывести на экран
    QSqlRecord rec = query.record();
    const int loginIndex = rec.indexOf("login");//номер "столбца"
    const int passwordIndex = rec.indexOf("password");

    while(query.next())
        qDebug()<<query.value(loginIndex).toString()
               <<"\t"<<query.value(passwordIndex)<<"\n";

    //закрыть соединение
    //удалить таблицу
   // query.exec("DROP TABLE User");
    //закрываем базу данных
    db.close();
    return a.exec();
}
