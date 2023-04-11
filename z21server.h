#ifndef Z21SERVER_H
#define Z21SERVER_H

#include <QObject>
#include <QHostAddress>
#include <QVarLengthArray>

#include "z21server_constants.h"

class QUdpSocket;
class z21Class;

extern "C" void notifyz21getSystemInfo(uint8_t client);
extern "C" void notifyz21EthSend(uint8_t client, uint8_t *data);
extern "C" void notifyz21RailPower(uint8_t State);
extern "C" void notifyz21S88Data(uint8_t group);
extern "C" uint8_t notifyz21ClientHash(uint8_t client);

class Z21Server : public QObject
{
    Q_OBJECT
public:
    explicit Z21Server(QObject *parent = nullptr);
    ~Z21Server();

    bool startServer(quint16 port = Z21::defaultServerPort);

    void setPower(Z21::PowerState state);
    Z21::PowerState getPower() const;

    uint8_t getS88State(int module) const;
    void setS88ModuleState(int module, uint8_t state);

signals:
    void powerStateChanged(int state);

private slots:
    void readPendingDatagram();

private:
    struct Client;
    int addClientAndGetIndex(const Client& client);
    void sendDatagram(int clientIdx, const char *data, const qint64 size);
    quint8 getClientHash(int clientIdx);
    void sendS88Status(int group);

private:
    friend void ::notifyz21getSystemInfo(uint8_t client);
    friend void notifyz21EthSend(uint8_t client, uint8_t *data);
    friend void notifyz21RailPower(uint8_t State);
    friend void notifyz21S88Data(uint8_t group);
    friend uint8_t notifyz21ClientHash(uint8_t client);

    z21Class *m_z21;

private:
    QUdpSocket *m_server;

    struct Client
    {
        QHostAddress remoteAddr;
        quint16 remotePort = 0;

        inline bool operator==(const Client& other) const
        {
            return remoteAddr == other.remoteAddr && remotePort == other.remotePort;
        }
    };
    QVarLengthArray<Client, 64> m_clients;

    uint8_t S88ModuleState[Z21::S88_MODULE_COUNT] = {};
};

#endif // Z21SERVER_H
