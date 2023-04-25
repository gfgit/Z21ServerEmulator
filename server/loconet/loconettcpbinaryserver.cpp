#include "loconettcpbinaryserver.h"

#ifdef WITH_LOCONET2

#include <QTcpServer>
#include <QTcpSocket>

#include "loconetbus.h"
#include "loconettcpsource.h"

LocoNetTCPBinaryServer::LocoNetTCPBinaryServer(LocoNetBusHolder *bus, QObject *parent) :
    QObject{parent}
{
    m_source = new LocoNetTcpSource(&bus->bus, this);

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection,
            this, &LocoNetTCPBinaryServer::handleNewClient);
}

bool LocoNetTCPBinaryServer::startServer(int port)
{
    return m_tcpServer->listen(QHostAddress::Any, port);
}

bool LocoNetTCPBinaryServer::broadcastMessage(const uint8_t *msg, int size)
{
    const char * msg_data = reinterpret_cast<const char *>(msg);
    bool success = true;

    for(Client &client : m_clients)
    {
        qint64 sz = client.sock->write(msg_data, size);
        if(sz != size)
            success = false;
    }

    return success;
}

void LocoNetTCPBinaryServer::handleNewClient()
{
    if(!m_tcpServer->hasPendingConnections())
        return;

    Client client;
    client.sock = m_tcpServer->nextPendingConnection();

    connect(client.sock, &QTcpSocket::disconnected, this, [this, sock=client.sock]()
            {
                for(int i = 0; i < m_clients.size(); i++)
                {
                    if(m_clients.at(i).sock == sock)
                    {
                        m_clients.removeAt(i);
                        break;
                    }
                }
                sock->deleteLater();
            });

    connect(client.sock, &QTcpSocket::readyRead, this, [this, sock=client.sock]()
            {
                handleSockData(sock);
            });

    m_clients.append(client);
}

void LocoNetTCPBinaryServer::handleSockData(QTcpSocket *sock)
{
    Client *client = nullptr;

    for(int i = 0; i < m_clients.size(); i++)
    {
        if(m_clients.at(i).sock == sock)
        {
            client = &m_clients[i];
            break;
        }
    }

    if(!client)
        return;

    uint8_t b = 0;
    while(sock->getChar(reinterpret_cast<char *>(&b)))
    {
        LnMsg *msg = client->buf.addByte(b);
        if(msg)
        {
            //Send packet for processing
            m_source->sendRaw(*msg);

            //Echo back to client to tell message arrived succesfully
            //NOTE: seems to be the way LBServer works and Traintastic expects it
            sock->write(reinterpret_cast<const char *>(msg->data), msg->length());
        }
    }
}

#endif // WITH_LOCONET2
