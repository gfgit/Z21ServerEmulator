#include "mainwindow.h"

#include <QApplication>
#include <QPointer>
#include <QThread>

#include "server/z21server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPointer<MainWindow> w = new MainWindow;
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();

    QThread z21Thread;
    QObject::connect(w, &QWidget::destroyed, &z21Thread, &QThread::quit);

    Z21Server server;
    server.moveToThread(&z21Thread);
    QObject::connect(&z21Thread, &QThread::started, &server, [&server]() { server.startServer(); });
    w->setupConnections(&server);

    z21Thread.start();

    int ret = a.exec();

    if (w)
        delete w;

    z21Thread.quit();
    z21Thread.wait();

    return ret;
}
