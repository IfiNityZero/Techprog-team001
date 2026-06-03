#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

/**
 * @file clientmanager.h
 * @brief Singleton-менеджер для управления несколькими окнами клиента.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 *
 * Реализует паттерн Singleton для управления окнами ClientWindow.
 * Позволяет создавать несколько независимых сессий с одного устройства,
 * каждая со своим подключением и авторизацией.
 */

#include <QObject>
#include <QList>

class ClientWindow;

/**
 * @brief Singleton-менеджер окон клиента.
 *
 * Управляет списком всех открытых окон ClientWindow.
 * Является единственной точкой создания новых окон.
 *
 * Паттерн Singleton гарантирует что менеджер существует
 * в единственном экземпляре, при этом окон может быть несколько.
 */
class ClientManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Получить единственный экземпляр менеджера.
     * @return Указатель на ClientManager
     */
    static ClientManager* getInstance();

    /**
     * @brief Создать новое окно клиента.
     *
     * Создаёт новый ClientWindow, добавляет в список,
     * подключает сигнал закрытия и показывает окно.
     *
     * @return Указатель на созданное окно
     */
    ClientWindow* createNewWindow();

    /**
     * @brief Получить количество открытых окон.
     * @return Число активных окон
     */
    int windowCount() const;

private slots:
    /**
     * @brief Слот удаления окна из списка при его закрытии.
     * @param window Указатель на закрытое окно
     */
    void onWindowClosed(ClientWindow *window);

private:
    explicit ClientManager(QObject *parent = nullptr);
    ClientManager(const ClientManager&) = delete;
    ClientManager& operator=(const ClientManager&) = delete;

    static ClientManager* p_instance; ///< Единственный экземпляр менеджера
    QList<ClientWindow*> mWindows;    ///< Список всех открытых окон
};

#endif // CLIENTMANAGER_H
