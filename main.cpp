#include "mainwindow.h"

#include <QApplication>

#include "z21server.h"

//TODO: move to common utils header
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

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

    QObject::connect(&w, &MainWindow::s88_state,
                     &server, [&server](int module, int port, bool value)
                     {
                         uint8_t state = server.getS88State(module);
                         state = bitWrite(state, port, value);
                         server.setS88ModuleState(module, state);
                     });

    server.startServer();

    return a.exec();
}
