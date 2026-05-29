#include "clientwindow.h"
#include <QApplication>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected,    this, &ClientWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ClientWindow::onDisconnected);
    connect(socket, &QTcpSocket::readyRead,    this, &ClientWindow::onReadyRead);

    setupUI();
    setWindowTitle("TaMP Client — команда 001");
    resize(640, 520);
}

ClientWindow::~ClientWindow() {}

// ─────────────────────────────────────────────
//  Построение интерфейса
// ─────────────────────────────────────────────
void ClientWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ── Подключение ──
    QGroupBox *grpConn = new QGroupBox("Подключение к серверу");
    QHBoxLayout *connLayout = new QHBoxLayout(grpConn);

    connLayout->addWidget(new QLabel("Хост:"));
    editHost = new QLineEdit("127.0.0.1");
    editHost->setFixedWidth(130);
    connLayout->addWidget(editHost);

    connLayout->addWidget(new QLabel("Порт:"));
    editPort = new QLineEdit("33333");
    editPort->setFixedWidth(70);
    connLayout->addWidget(editPort);

    btnConnect = new QPushButton("Подключиться");
    btnDisconnect = new QPushButton("Отключиться");
    btnDisconnect->setEnabled(false);
    connLayout->addWidget(btnConnect);
    connLayout->addWidget(btnDisconnect);
    connLayout->addStretch();

    labelStatus = new QLabel("● Не подключён");
    labelStatus->setStyleSheet("color: gray; font-weight: bold;");
    connLayout->addWidget(labelStatus);

    mainLayout->addWidget(grpConn);

    // ── Команда ──
    QGroupBox *grpCmd = new QGroupBox("Команда");
    QVBoxLayout *cmdLayout = new QVBoxLayout(grpCmd);

    QHBoxLayout *row1 = new QHBoxLayout();
    row1->addWidget(new QLabel("Операция:"));
    comboCommand = new QComboBox();
    comboCommand->addItem("Шифр Виженера — зашифровать", "vigenere_encrypt");
    comboCommand->addItem("Шифр Виженера — расшифровать", "vigenere_decrypt");
    comboCommand->addItem("SHA-384 хеш", "sha384");
    comboCommand->addItem("Метод хорд (корень уравнения)", "chord");
    comboCommand->addItem("Стеганография — внедрить", "stego_encode");
    comboCommand->addItem("Стеганография — извлечь", "stego_decode");
    row1->addWidget(comboCommand);
    row1->addStretch();
    cmdLayout->addLayout(row1);

    // Поля аргументов
    QHBoxLayout *row2 = new QHBoxLayout();
    labelArg1 = new QLabel("Текст:");
    editArg1  = new QLineEdit();
    editArg1->setPlaceholderText("Введите текст...");
    row2->addWidget(labelArg1);
    row2->addWidget(editArg1);
    cmdLayout->addLayout(row2);

    QHBoxLayout *row3 = new QHBoxLayout();
    labelArg2 = new QLabel("Ключ:");
    editArg2  = new QLineEdit();
    editArg2->setPlaceholderText("Введите ключ...");
    row3->addWidget(labelArg2);
    row3->addWidget(editArg2);
    cmdLayout->addLayout(row3);

    QHBoxLayout *row4 = new QHBoxLayout();
    labelArg3 = new QLabel("Точность:");
    editArg3  = new QLineEdit("0.000001");
    row4->addWidget(labelArg3);
    row4->addWidget(editArg3);
    cmdLayout->addLayout(row4);

    btnSend = new QPushButton("▶  Отправить");
    btnSend->setEnabled(false);
    btnSend->setFixedHeight(36);
    cmdLayout->addWidget(btnSend);

    mainLayout->addWidget(grpCmd);

    // ── Лог ──
    QGroupBox *grpLog = new QGroupBox("Результат");
    QVBoxLayout *logLayout = new QVBoxLayout(grpLog);

    textLog = new QTextEdit();
    textLog->setReadOnly(true);
    textLog->setFont(QFont("Courier New", 10));
    logLayout->addWidget(textLog);

    btnClear = new QPushButton("Очистить лог");
    btnClear->setFixedWidth(120);
    logLayout->addWidget(btnClear, 0, Qt::AlignRight);

    mainLayout->addWidget(grpLog);

    // ── Сигналы ──
    connect(btnConnect,    &QPushButton::clicked, this, &ClientWindow::connectToServer);
    connect(btnDisconnect, &QPushButton::clicked, this, &ClientWindow::disconnectFromServer);
    connect(btnSend,       &QPushButton::clicked, this, &ClientWindow::sendCommand);
    connect(btnClear,      &QPushButton::clicked, textLog, &QTextEdit::clear);
    connect(comboCommand,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClientWindow::onCommandChanged);

    // Начальное состояние полей
    onCommandChanged(0);
}

// ─────────────────────────────────────────────
//  Адаптация полей под выбранную команду
// ─────────────────────────────────────────────
void ClientWindow::onCommandChanged(int index)
{
    QString cmd = comboCommand->itemData(index).toString();

    // Сброс видимости
    labelArg1->show(); editArg1->show();
    labelArg2->show(); editArg2->show();
    labelArg3->show(); editArg3->show();

    if (cmd == "vigenere_encrypt" || cmd == "vigenere_decrypt") {
        labelArg1->setText("Текст:");
        editArg1->setPlaceholderText("Например: HELLO");
        labelArg2->setText("Ключ:");
        editArg2->setPlaceholderText("Например: KEY");
        labelArg3->hide(); editArg3->hide();
    }
    else if (cmd == "sha384") {
        labelArg1->setText("Текст:");
        editArg1->setPlaceholderText("Любая строка...");
        labelArg2->hide(); editArg2->hide();
        labelArg3->hide(); editArg3->hide();
    }
    else if (cmd == "chord") {
        labelArg1->setText("Левая граница a:");
        editArg1->setPlaceholderText("Например: 1");
        labelArg2->setText("Правая граница b:");
        editArg2->setPlaceholderText("Например: 2");
        labelArg3->setText("Точность eps:");
        editArg3->setText("0.000001");
    }
    else if (cmd == "stego_encode") {
        labelArg1->setText("Путь к BMP:");
        editArg1->setPlaceholderText("/path/to/image.bmp");
        labelArg2->setText("Сообщение:");
        editArg2->setPlaceholderText("Текст для внедрения...");
        labelArg3->hide(); editArg3->hide();
    }
    else if (cmd == "stego_decode") {
        labelArg1->setText("Путь к BMP:");
        editArg1->setPlaceholderText("/path/to/image.bmp");
        labelArg2->hide(); editArg2->hide();
        labelArg3->hide(); editArg3->hide();
    }
}

// ─────────────────────────────────────────────
//  Сеть
// ─────────────────────────────────────────────
void ClientWindow::connectToServer()
{
    QString host = editHost->text().trimmed();
    int port = editPort->text().toInt();
    appendLog(QString("Подключение к %1:%2...").arg(host).arg(port));
    socket->connectToHost(host, port);
}

void ClientWindow::disconnectFromServer()
{
    socket->disconnectFromHost();
}

void ClientWindow::onConnected()
{
    labelStatus->setText("● Подключён");
    labelStatus->setStyleSheet("color: green; font-weight: bold;");
    btnConnect->setEnabled(false);
    btnDisconnect->setEnabled(true);
    btnSend->setEnabled(true);
    appendLog("✓ Подключение установлено", "green");
}

void ClientWindow::onDisconnected()
{
    labelStatus->setText("● Не подключён");
    labelStatus->setStyleSheet("color: gray; font-weight: bold;");
    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);
    btnSend->setEnabled(false);
    appendLog("✗ Соединение разорвано", "red");
}

void ClientWindow::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data).trimmed();
    appendLog("◀  Ответ: " + response, "blue");
}

// ─────────────────────────────────────────────
//  Отправка команды
// ─────────────────────────────────────────────
void ClientWindow::sendCommand()
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        appendLog("Ошибка: нет подключения к серверу", "red");
        return;
    }

    int idx = comboCommand->currentIndex();
    QString cmd = comboCommand->itemData(idx).toString();
    QString arg1 = editArg1->text().trimmed();
    QString arg2 = editArg2->text().trimmed();
    QString arg3 = editArg3->text().trimmed();

    QString request;

    if (cmd == "vigenere_encrypt" || cmd == "vigenere_decrypt") {
        if (arg1.isEmpty() || arg2.isEmpty()) {
            appendLog("Ошибка: заполните текст и ключ", "red"); return;
        }
        request = QString("%1:%2:%3").arg(cmd, arg1, arg2);
    }
    else if (cmd == "sha384") {
        if (arg1.isEmpty()) {
            appendLog("Ошибка: введите текст", "red"); return;
        }
        request = QString("sha384:%1").arg(arg1);
    }
    else if (cmd == "chord") {
        if (arg1.isEmpty() || arg2.isEmpty()) {
            appendLog("Ошибка: введите границы отрезка", "red"); return;
        }
        request = QString("chord:%1:%2:%3").arg(arg1, arg2, arg3);
    }
    else if (cmd == "stego_encode") {
        if (arg1.isEmpty() || arg2.isEmpty()) {
            appendLog("Ошибка: укажите путь и сообщение", "red"); return;
        }
        request = QString("stego_encode:%1:%2").arg(arg1, arg2);
    }
    else if (cmd == "stego_decode") {
        if (arg1.isEmpty()) {
            appendLog("Ошибка: укажите путь к файлу", "red"); return;
        }
        request = QString("stego_decode:%1").arg(arg1);
    }

    appendLog("▶  Отправка: " + request);
    socket->write((request + "\r\n").toUtf8());
}

// ─────────────────────────────────────────────
//  Лог
// ─────────────────────────────────────────────
void ClientWindow::appendLog(const QString &msg, const QString &color)
{
    if (color.isEmpty())
        textLog->append(msg);
    else
        textLog->append(QString("<span style='color:%1'>%2</span>").arg(color, msg.toHtmlEscaped()));
}
