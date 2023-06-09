#include "mainwindow.h"

#include <QApplication>

#include "server/z21server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Z21Server server;
    w.setupConnections(&server);

    server.startServer();

    return a.exec();
}
