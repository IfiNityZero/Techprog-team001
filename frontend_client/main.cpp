/**
 * @file main.cpp
 * @brief Точка входа клиентского приложения.
 *
 * Использует ClientManager (Singleton) для создания первого окна.
 * Дополнительные окна создаются кнопкой "+ Новая сессия".
 */

#include <QApplication>
#include "clientmanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TaMP Client");

    // получаем Singleton менеджер и создаём первое окно
    ClientManager::getInstance()->createNewWindow();

    return app.exec(); // запускаем Qt Event Loop
}
