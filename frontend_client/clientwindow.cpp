#include "clientwindow.h"
#include <QApplication>

// Инициализация статического указателя
ClientWindow* ClientWindow::p_instance = nullptr;

ClientWindow* ClientWindow::getInstance()
{
    if (!p_instance)
        p_instance = new ClientWindow();
    return p_instance;
}

void ClientWindow::dropInstance()
{
    delete p_instance;
    p_instance = nullptr;
}

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected,    this, &ClientWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ClientWindow::onDisconnected);
    connect(socket, &QTcpSocket::readyRead,    this, &ClientWindow::onReadyRead);

    setupUI();
    setWindowTitle("TaMP Client — команда 001");
    resize(800, 600);
}

ClientWindow::~ClientWindow() {}

void ClientWindow::setupUI()
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginPage();
    setupMainPage();

    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(mainPage);
    stackedWidget->setCurrentWidget(loginPage);
}

void ClientWindow::setupLoginPage()
{
    loginPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(loginPage);
    layout->setSpacing(10);
    layout->setContentsMargins(40, 40, 40, 40);

    QLabel *title = new QLabel("TaMP Client — команда 001");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(title);

    QGroupBox *grpConn = new QGroupBox("Подключение к серверу");
    QHBoxLayout *connLayout = new QHBoxLayout(grpConn);
    connLayout->addWidget(new QLabel("Хост:"));
    editHost = new QLineEdit("127.0.0.1");
    editHost->setFixedWidth(140);
    connLayout->addWidget(editHost);
    connLayout->addWidget(new QLabel("Порт:"));
    editPort = new QLineEdit("33333");
    editPort->setFixedWidth(70);
    connLayout->addWidget(editPort);
    btnConnect = new QPushButton("Подключиться");
    connLayout->addWidget(btnConnect);
    labelConnStatus = new QLabel("● Не подключён");
    labelConnStatus->setStyleSheet("color: gray; font-weight: bold;");
    connLayout->addWidget(labelConnStatus);
    connLayout->addStretch();
    layout->addWidget(grpConn);

    QGroupBox *grpAuth = new QGroupBox("Авторизация");
    QVBoxLayout *authLayout = new QVBoxLayout(grpAuth);

    QHBoxLayout *rowLogin = new QHBoxLayout();
    rowLogin->addWidget(new QLabel("Логин:"));
    editLogin = new QLineEdit();
    editLogin->setPlaceholderText("Введите логин...");
    rowLogin->addWidget(editLogin);
    authLayout->addLayout(rowLogin);

    QHBoxLayout *rowPass = new QHBoxLayout();
    rowPass->addWidget(new QLabel("Пароль:"));
    editPassword = new QLineEdit();
    editPassword->setPlaceholderText("Введите пароль...");
    editPassword->setEchoMode(QLineEdit::Password);
    rowPass->addWidget(editPassword);
    authLayout->addLayout(rowPass);

    QHBoxLayout *rowBtns = new QHBoxLayout();
    btnLogin    = new QPushButton("Войти");
    btnRegister = new QPushButton("Зарегистрироваться");
    btnLogin->setEnabled(false);
    btnRegister->setEnabled(false);
    btnLogin->setFixedHeight(36);
    btnRegister->setFixedHeight(36);
    rowBtns->addWidget(btnLogin);
    rowBtns->addWidget(btnRegister);
    authLayout->addLayout(rowBtns);

    labelAuthStatus = new QLabel("");
    labelAuthStatus->setAlignment(Qt::AlignCenter);
    authLayout->addWidget(labelAuthStatus);
    layout->addWidget(grpAuth);
    layout->addStretch();

    QLabel *hint = new QLabel("Администратор по умолчанию: login=admin, password=admin123");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: gray; font-size: 11px;");
    layout->addWidget(hint);

    connect(btnConnect,  &QPushButton::clicked, this, &ClientWindow::connectToServer);
    connect(btnLogin,    &QPushButton::clicked, this, &ClientWindow::sendLogin);
    connect(btnRegister, &QPushButton::clicked, this, &ClientWindow::sendRegister);
}

void ClientWindow::setupMainPage()
{
    mainPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(mainPage);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    QHBoxLayout *topRow = new QHBoxLayout();
    labelUserInfo = new QLabel("Пользователь: —");
    labelUserInfo->setStyleSheet("font-weight: bold; font-size: 13px;");
    topRow->addWidget(labelUserInfo);
    topRow->addStretch();
    btnLogout = new QPushButton("Выйти");
    btnLogout->setFixedWidth(80);
    topRow->addWidget(btnLogout);
    layout->addLayout(topRow);

    QGroupBox *grpCmd = new QGroupBox("Команда");
    QVBoxLayout *cmdLayout = new QVBoxLayout(grpCmd);

    QHBoxLayout *row1 = new QHBoxLayout();
    row1->addWidget(new QLabel("Операция:"));
    comboCommand = new QComboBox();
    comboCommand->setMinimumWidth(280);
    row1->addWidget(comboCommand);
    row1->addStretch();
    cmdLayout->addLayout(row1);

    QHBoxLayout *row2 = new QHBoxLayout();
    labelArg1 = new QLabel("Текст:");
    editArg1  = new QLineEdit();
    row2->addWidget(labelArg1);
    row2->addWidget(editArg1);
    cmdLayout->addLayout(row2);

    QHBoxLayout *row3 = new QHBoxLayout();
    labelArg2 = new QLabel("Ключ:");
    editArg2  = new QLineEdit();
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
    btnSend->setFixedHeight(36);
    cmdLayout->addWidget(btnSend);
    layout->addWidget(grpCmd);

    QGroupBox *grpTable = new QGroupBox("Результаты");
    QVBoxLayout *tableLayout = new QVBoxLayout(grpTable);
    tableResult = new QTableWidget(0, 2);
    tableResult->setHorizontalHeaderLabels({"Команда", "Ответ"});
    tableResult->horizontalHeader()->setStretchLastSection(true);
    tableResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableResult->setAlternatingRowColors(true);
    tableLayout->addWidget(tableResult);
    layout->addWidget(grpTable);

    QGroupBox *grpLog = new QGroupBox("Лог");
    QVBoxLayout *logLayout = new QVBoxLayout(grpLog);
    textLog = new QTextEdit();
    textLog->setReadOnly(true);
    textLog->setMaximumHeight(120);
    textLog->setFont(QFont("Courier New", 9));
    logLayout->addWidget(textLog);
    btnClear = new QPushButton("Очистить");
    btnClear->setFixedWidth(100);
    logLayout->addWidget(btnClear, 0, Qt::AlignRight);
    layout->addWidget(grpLog);

    connect(btnSend,      &QPushButton::clicked, this, &ClientWindow::sendCommand);
    connect(btnLogout,    &QPushButton::clicked, this, &ClientWindow::showLoginPage);
    connect(btnClear,     &QPushButton::clicked, textLog, &QTextEdit::clear);
    connect(comboCommand, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClientWindow::onCommandChanged);
}

void ClientWindow::showLoginPage()
{
    currentRole = "";
    stackedWidget->setCurrentWidget(loginPage);
    labelAuthStatus->setText("");
}

void ClientWindow::showMainPage(const QString &role)
{
    currentRole = role;
    comboCommand->clear();
    comboCommand->addItem("Шифр Виженера — зашифровать", "vigenere_encrypt");
    comboCommand->addItem("Шифр Виженера — расшифровать", "vigenere_decrypt");
    comboCommand->addItem("SHA-384 хеш", "sha384");
    comboCommand->addItem("Метод хорд", "chord");
    comboCommand->addItem("Стеганография — внедрить", "stego_encode");
    comboCommand->addItem("Стеганография — извлечь", "stego_decode");

    if (role == "admin") {
        comboCommand->addItem("[ADMIN] Список пользователей", "admin_users");
        comboCommand->addItem("[ADMIN] Удалить пользователя", "admin_delete");
        labelUserInfo->setText("👑 Администратор: " + editLogin->text());
        labelUserInfo->setStyleSheet("font-weight: bold; font-size: 13px; color: orange;");
    } else {
        labelUserInfo->setText("👤 Пользователь: " + editLogin->text());
        labelUserInfo->setStyleSheet("font-weight: bold; font-size: 13px; color: green;");
    }

    stackedWidget->setCurrentWidget(mainPage);
    onCommandChanged(0);
}

void ClientWindow::connectToServer()
{
    QString host = editHost->text().trimmed();
    int port = editPort->text().toInt();
    labelConnStatus->setText("● Подключение...");
    labelConnStatus->setStyleSheet("color: orange; font-weight: bold;");
    socket->connectToHost(host, port);
}

void ClientWindow::disconnectFromServer()
{
    socket->disconnectFromHost();
}

void ClientWindow::onConnected()
{
    labelConnStatus->setText("● Подключён");
    labelConnStatus->setStyleSheet("color: green; font-weight: bold;");
    btnConnect->setEnabled(false);
    btnLogin->setEnabled(true);
    btnRegister->setEnabled(true);
    labelAuthStatus->setText("Введите логин и пароль");
    labelAuthStatus->setStyleSheet("color: gray;");
}

void ClientWindow::onDisconnected()
{
    labelConnStatus->setText("● Не подключён");
    labelConnStatus->setStyleSheet("color: gray; font-weight: bold;");
    btnConnect->setEnabled(true);
    btnLogin->setEnabled(false);
    btnRegister->setEnabled(false);
    showLoginPage();
}

void ClientWindow::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data).trimmed();
    appendLog("◀ " + response);

    if (pendingAction == "login") {
        pendingAction = "";
        if (response.startsWith("ok: logged in as")) {
            QString role = response.contains("admin") ? "admin" : "user";
            showMainPage(role);
        } else {
            labelAuthStatus->setText(response);
            labelAuthStatus->setStyleSheet("color: red;");
        }
        return;
    }

    if (pendingAction == "register") {
        pendingAction = "";
        labelAuthStatus->setText(response);
        labelAuthStatus->setStyleSheet(
            response.startsWith("ok") ? "color: green;" : "color: red;");
        return;
    }

    if (stackedWidget->currentWidget() == mainPage)
        addTableRow(lastCommand, response);
}

void ClientWindow::sendLogin()
{
    QString login = editLogin->text().trimmed();
    QString pass  = editPassword->text().trimmed();
    if (login.isEmpty() || pass.isEmpty()) {
        labelAuthStatus->setText("Заполните логин и пароль");
        labelAuthStatus->setStyleSheet("color: red;");
        return;
    }
    pendingAction = "login";
    QString cmd = "login:" + login + ":" + pass;
    appendLog("▶ " + cmd);
    socket->write((cmd + "\r\n").toUtf8());
}

void ClientWindow::sendRegister()
{
    QString login = editLogin->text().trimmed();
    QString pass  = editPassword->text().trimmed();
    if (login.isEmpty() || pass.isEmpty()) {
        labelAuthStatus->setText("Заполните логин и пароль");
        labelAuthStatus->setStyleSheet("color: red;");
        return;
    }
    pendingAction = "register";
    QString cmd = "register:" + login + ":" + pass;
    appendLog("▶ " + cmd);
    socket->write((cmd + "\r\n").toUtf8());
}

void ClientWindow::sendCommand()
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        appendLog("Ошибка: нет подключения"); return;
    }

    int idx = comboCommand->currentIndex();
    QString cmd = comboCommand->itemData(idx).toString();
    QString arg1 = editArg1->text().trimmed();
    QString arg2 = editArg2->text().trimmed();
    QString arg3 = editArg3->text().trimmed();
    QString request;

    if (cmd == "vigenere_encrypt" || cmd == "vigenere_decrypt") {
        if (arg1.isEmpty() || arg2.isEmpty()) { appendLog("Ошибка: текст и ключ"); return; }
        request = cmd + ":" + arg1 + ":" + arg2;
    } else if (cmd == "sha384") {
        if (arg1.isEmpty()) { appendLog("Ошибка: введите текст"); return; }
        request = "sha384:" + arg1;
    } else if (cmd == "chord") {
        if (arg1.isEmpty() || arg2.isEmpty()) { appendLog("Ошибка: границы"); return; }
        request = "chord:" + arg1 + ":" + arg2 + ":" + arg3;
    } else if (cmd == "stego_encode") {
        if (arg1.isEmpty() || arg2.isEmpty()) { appendLog("Ошибка: путь и сообщение"); return; }
        request = "stego_encode:" + arg1 + ":" + arg2;
    } else if (cmd == "stego_decode") {
        if (arg1.isEmpty()) { appendLog("Ошибка: путь"); return; }
        request = "stego_decode:" + arg1;
    } else if (cmd == "admin_users") {
        request = "admin_users";
    } else if (cmd == "admin_delete") {
        if (arg1.isEmpty()) { appendLog("Ошибка: логин"); return; }
        request = "admin_delete:" + arg1;
    }

    lastCommand = comboCommand->currentText();
    appendLog("▶ " + request);
    socket->write((request + "\r\n").toUtf8());
}

void ClientWindow::onCommandChanged(int index)
{
    QString cmd = comboCommand->itemData(index).toString();
    labelArg1->show(); editArg1->show();
    labelArg2->show(); editArg2->show();
    labelArg3->show(); editArg3->show();
    editArg1->clear(); editArg2->clear();

    if (cmd == "vigenere_encrypt" || cmd == "vigenere_decrypt") {
        labelArg1->setText("Текст:"); editArg1->setPlaceholderText("Например: HELLO");
        labelArg2->setText("Ключ:");  editArg2->setPlaceholderText("Например: KEY");
        labelArg3->hide(); editArg3->hide();
    } else if (cmd == "sha384") {
        labelArg1->setText("Текст:"); editArg1->setPlaceholderText("Любая строка...");
        labelArg2->hide(); editArg2->hide();
        labelArg3->hide(); editArg3->hide();
    } else if (cmd == "chord") {
        labelArg1->setText("Граница a:"); editArg1->setPlaceholderText("1");
        labelArg2->setText("Граница b:"); editArg2->setPlaceholderText("2");
        labelArg3->setText("Точность:"); editArg3->setText("0.000001");
    } else if (cmd == "stego_encode") {
        labelArg1->setText("Путь BMP:"); editArg1->setPlaceholderText("/app/test.bmp");
        labelArg2->setText("Сообщение:"); editArg2->setPlaceholderText("Текст...");
        labelArg3->hide(); editArg3->hide();
    } else if (cmd == "stego_decode") {
        labelArg1->setText("Путь BMP:"); editArg1->setPlaceholderText("/app/test.bmp");
        labelArg2->hide(); editArg2->hide();
        labelArg3->hide(); editArg3->hide();
    } else if (cmd == "admin_users") {
        labelArg1->hide(); editArg1->hide();
        labelArg2->hide(); editArg2->hide();
        labelArg3->hide(); editArg3->hide();
    } else if (cmd == "admin_delete") {
        labelArg1->setText("Логин:"); editArg1->setPlaceholderText("Логин пользователя");
        labelArg2->hide(); editArg2->hide();
        labelArg3->hide(); editArg3->hide();
    }
}

void ClientWindow::addTableRow(const QString &cmd, const QString &response)
{
    int row = tableResult->rowCount();
    tableResult->insertRow(row);
    tableResult->setItem(row, 0, new QTableWidgetItem(cmd));
    tableResult->setItem(row, 1, new QTableWidgetItem(response));
    tableResult->scrollToBottom();
}

void ClientWindow::appendLog(const QString &msg, const QString &color)
{
    if (color.isEmpty())
        textLog->append(msg);
    else
        textLog->append(QString("<span style='color:%1'>%2</span>")
                        .arg(color, msg.toHtmlEscaped()));
}
