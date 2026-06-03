/**
 * @file clientmanager.cpp
 * @brief Реализация Singleton-менеджера окон клиента.
 */

#include "clientmanager.h"
#include "clientwindow.h"
#include <QDebug>

// Инициализация статического указателя
ClientManager* ClientManager::p_instance = nullptr;

ClientManager* ClientManager::getInstance()
{
    if (!p_instance)                      // если менеджер ещё не создан
        p_instance = new ClientManager(); // создаём один раз
    return p_instance;
}

ClientManager::ClientManager(QObject *parent) : QObject(parent)
{
    qDebug() << "ClientManager Singleton created";
}

ClientWindow* ClientManager::createNewWindow()
{
    // создаём новое независимое окно клиента
    ClientWindow *window = new ClientWindow();

    // подключаем сигнал закрытия окна к нашему слоту
    connect(window, &ClientWindow::windowClosed,
            this, &ClientManager::onWindowClosed);

    mWindows.append(window); // добавляем в список окон

    window->setWindowTitle(
        QString("TaMP Client — сессия %1").arg(mWindows.size())
    ); // нумеруем окна

    window->show(); // показываем окно

    qDebug() << "New window created. Total windows:" << mWindows.size();

    return window;
}

int ClientManager::windowCount() const
{
    return mWindows.size(); // возвращаем количество открытых окон
}

void ClientManager::onWindowClosed(ClientWindow *window)
{
    mWindows.removeAll(window); // удаляем закрытое окно из списка
    qDebug() << "Window closed. Remaining:" << mWindows.size();
}
