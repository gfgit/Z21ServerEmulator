#ifndef LOCONETTCPBINARYSERVER_H
#define LOCONETTCPBINARYSERVER_H

#ifdef WITH_LOCONET2

#include <QObject>
#include <QVector>

#include "LocoNetMessageBuffer.h"

class QTcpServer;
class QTcpSocket;

class LocoNetTcpSource;
struct LocoNetBusHolder;

class LocoNetTCPBinaryServer : public QObject
{
    Q_OBJECT
public:
    LocoNetTCPBinaryServer(LocoNetBusHolder *bus, QObject *parent = nullptr);

    bool startServer(int port = 5550);

    bool broadcastMessage(const uint8_t *msg, int size);

private slots:
    void handleNewClient();
    void handleSockData(QTcpSocket *sock);

private:
    LocoNetTcpSource *m_source;

    QTcpServer *m_server;

    struct Client
    {
        QTcpSocket *sock;
        LocoNetMessageBuffer buf;
    };

    QVector<Client> m_clients;
};

#endif // WITH_LOCONET2

#endif // LOCONETTCPBINARYSERVER_H
