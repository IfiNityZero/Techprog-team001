#include <QApplication>
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TaMP Client");

    // Получаем единственный экземпляр через Singleton
    ClientWindow *window = ClientWindow::getInstance();
    window->show();

    int result = app.exec();

    // Удаляем экземпляр при закрытии
    ClientWindow::dropInstance();

    return result;
}
