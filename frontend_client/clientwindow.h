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

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void connectToServer();
    void disconnectFromServer();
    void sendCommand();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onCommandChanged(int index);

private:
    void setupUI();
    void appendLog(const QString &msg, const QString &color = "");

    // Сеть
    QTcpSocket *socket;

    // UI элементы
    QLineEdit   *editHost;
    QLineEdit   *editPort;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect;

    QComboBox   *comboCommand;
    QLineEdit   *editArg1;
    QLineEdit   *editArg2;
    QLineEdit   *editArg3;
    QLabel      *labelArg1;
    QLabel      *labelArg2;
    QLabel      *labelArg3;
    QPushButton *btnSend;

    QTextEdit   *textLog;
    QPushButton *btnClear;

    QLabel      *labelStatus;
};

#endif // CLIENTWINDOW_H
