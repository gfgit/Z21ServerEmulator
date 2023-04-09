#ifndef Z21SERVER_H
#define Z21SERVER_H

#include <QObject>
#include <QHostAddress>
#include <QVarLengthArray>

#include "z21server_constants.h"

class QUdpSocket;
class z21Class;

class Z21Server : public QObject
{
    Q_OBJECT
public:
    explicit Z21Server(QObject *parent = nullptr);
    ~Z21Server();

    bool startServer(quint16 port = Z21::defaultServerPort);

    void setPower(Z21::PowerState state);
    Z21::PowerState getPower() const;

signals:
    void powerStateChanged(int state);

private slots:
    void readPendingDatagram();

private:
    struct Client;
    int addClientAndGetIndex(const Client& client);

public:
    void sendDatagram(int clientIdx, const char *data, const qint64 size);
    quint8 getClientHash(int clientIdx);

public:
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
};

#endif // Z21SERVER_H
