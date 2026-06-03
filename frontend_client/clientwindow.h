#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

/**
 * @file clientwindow.h
 * @brief Заголовочный файл окна клиента.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 *
 * Окно клиента с авторизацией, ролями и таблицей результатов.
 * Несколько экземпляров управляются через ClientManager (Singleton).
 */

#include <QMainWindow>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QCloseEvent>

/**
 * @brief Окно клиента для работы с TCP сервером.
 *
 * Содержит два экрана (через QStackedWidget):
 * - Экран авторизации: подключение, логин, регистрация
 * - Основной экран: команды, таблица результатов, лог
 *
 * Каждое окно — независимая сессия со своим сокетом и авторизацией.
 * Создаётся через ClientManager::createNewWindow().
 */
class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор окна клиента.
     * @param parent Родительский виджет (по умолчанию nullptr)
     */
    explicit ClientWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор.
     */
    ~ClientWindow();

signals:
    /**
     * @brief Сигнал испускается при закрытии окна.
     *
     * ClientManager подключается к этому сигналу
     * чтобы удалить окно из своего списка.
     *
     * @param window Указатель на закрытое окно
     */
    void windowClosed(ClientWindow *window);

protected:
    /**
     * @brief Перехватываем событие закрытия окна.
     *
     * Испускаем сигнал windowClosed перед закрытием.
     *
     * @param event Событие закрытия
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    void connectToServer();      ///< Подключиться к серверу
    void disconnectFromServer(); ///< Отключиться от сервера
    void sendLogin();            ///< Отправить запрос авторизации
    void sendRegister();         ///< Отправить запрос регистрации
    void sendCommand();          ///< Отправить команду серверу
    void onConnected();          ///< Обработка успешного подключения
    void onDisconnected();       ///< Обработка отключения
    void onReadyRead();          ///< Обработка входящих данных
    void onCommandChanged(int index); ///< Обновление полей при смене команды
    void openNewWindow();        ///< Открыть новое окно клиента

private:
    void setupUI();          ///< Построить интерфейс
    void setupLoginPage();   ///< Построить экран авторизации
    void setupMainPage();    ///< Построить основной экран
    void showLoginPage();    ///< Переключиться на экран авторизации
    void showMainPage(const QString &role); ///< Переключиться на основной экран

    /**
     * @brief Добавить сообщение в лог.
     * @param msg Текст сообщения
     * @param color Цвет текста (опционально)
     */
    void appendLog(const QString &msg, const QString &color = "");

    /**
     * @brief Добавить строку в таблицу результатов.
     * @param cmd Название команды
     * @param response Ответ сервера
     */
    void addTableRow(const QString &cmd, const QString &response);

    // Сеть
    QTcpSocket  *socket;         ///< TCP сокет для связи с сервером
    QString      currentRole;    ///< Роль текущего пользователя
    QString      pendingAction;  ///< Ожидаемое действие (login/register)
    QString      lastCommand;    ///< Последняя отправленная команда

    QStackedWidget *stackedWidget; ///< Контейнер страниц (авторизация / основной)

    // Страница авторизации
    QWidget     *loginPage;
    QLineEdit   *editHost;
    QLineEdit   *editPort;
    QPushButton *btnConnect;
    QLineEdit   *editLogin;
    QLineEdit   *editPassword;
    QPushButton *btnLogin;
    QPushButton *btnRegister;
    QLabel      *labelConnStatus;
    QLabel      *labelAuthStatus;

    // Основной экран
    QWidget      *mainPage;
    QLabel       *labelUserInfo;
    QComboBox    *comboCommand;
    QLineEdit    *editArg1;
    QLineEdit    *editArg2;
    QLineEdit    *editArg3;
    QLabel       *labelArg1;
    QLabel       *labelArg2;
    QLabel       *labelArg3;
    QPushButton  *btnSend;
    QPushButton  *btnLogout;
    QPushButton  *btnNewWindow; ///< Кнопка открытия нового окна
    QTextEdit    *textLog;
    QTableWidget *tableResult;
    QPushButton  *btnClear;
};

#endif // CLIENTWINDOW_H
