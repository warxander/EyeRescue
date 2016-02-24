#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("EyeRescue");
    QApplication::setOrganizationDomain("eyerescue.com");
    QApplication::setApplicationName("EyeRescue");

    app.setQuitOnLastWindowClosed(false);

    MainWindow window;
    window.show();

    return app.exec();
}
