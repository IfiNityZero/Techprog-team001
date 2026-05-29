#include <QApplication>
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TaMP Client");

    ClientWindow window;
    window.show();

    return app.exec();
}
