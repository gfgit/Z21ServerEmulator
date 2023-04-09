#include "mainwindow.h"

#include <QApplication>

#include "z21server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Z21Server server;
    server.startServer();

    return a.exec();
}
