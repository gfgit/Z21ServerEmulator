#include "mainwindow.h"

#include <QApplication>

#include "z21server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Z21Server server;

    QObject::connect(&server, &Z21Server::powerStateChanged,
                     &w, &MainWindow::setPowerStateLed);

    QObject::connect(&w, &MainWindow::setShortCircuit,
                     &server, [&server]()
                     {
                         server.setPower(Z21::PowerState::ShortCircuit);
                     });

    server.startServer();

    return a.exec();
}
