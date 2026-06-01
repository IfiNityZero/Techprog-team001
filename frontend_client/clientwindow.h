#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

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

/**
 * @brief Главное окно клиента с авторизацией и ролями.
 *
 * Содержит два экрана:
 * - Экран авторизации (логин/регистрация)
 * - Основной экран работы с сервером
 */
class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void connectToServer();
    void disconnectFromServer();
    void sendLogin();
    void sendRegister();
    void sendCommand();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onCommandChanged(int index);

private:
    void setupUI();
    void setupLoginPage();
    void setupMainPage();
    void showLoginPage();
    void showMainPage(const QString &role);
    void appendLog(const QString &msg, const QString &color = "");
    void addTableRow(const QString &cmd, const QString &response);

    // Сеть
    QTcpSocket  *socket;
    QString      currentRole;
    QString      pendingAction;
    QString      lastCommand;

    // Стек страниц
    QStackedWidget *stackedWidget;

    // Страница 1: логин
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

    // Страница 2: основной экран
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
    QTextEdit    *textLog;
    QTableWidget *tableResult;
    QPushButton  *btnClear;
};

#endif // CLIENTWINDOW_H
