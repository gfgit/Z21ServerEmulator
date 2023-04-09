#include "mainwindow.h"

#include <QApplication>

#include <QTimer>

#include "arduinocompat.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    arduino_setup();
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &timer, [](){ arduino_loop(); });
    timer.start(1000);

    return a.exec();
}
