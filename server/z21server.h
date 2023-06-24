#ifndef Z21SERVER_H
#define Z21SERVER_H

#include <QObject>
#include <QHostAddress>
#include <QVarLengthArray>

#include "z21server_constants.h"

class QUdpSocket;
class QTimer;

class z21Class;

class RBusRetroaction;
class AccessoryManager;
class LocoManager;

#ifdef WITH_LOCONET2
class LocoNetZ21Adapter;
#endif

extern "C" void notifyz21getSystemInfo(uint8_t client);
extern "C" void notifyz21EthSend(uint8_t client, uint8_t *data);
extern "C" void notifyz21RailPower(uint8_t State);
extern "C" uint8_t notifyz21ClientHash(uint8_t client);
extern "C" void notifyz21LNdetector(uint8_t client, uint8_t typ, uint16_t Adr);

class Z21Server : public QObject
{
    Q_OBJECT
public:
    explicit Z21Server(QObject *parent = nullptr);
    ~Z21Server();

    bool startServer(quint16 port = Z21::defaultServerPort);

    void setPower(Z21::PowerState state);
    Z21::PowerState getPower() const;

    RBusRetroaction *getRBUS() const;

    AccessoryManager *getAccessoryMgr() const;

    LocoManager *getLocoMgr() const;

#ifdef WITH_LOCONET2
    LocoNetZ21Adapter *getLocoNetAdapter() const;
#endif

signals:
    void powerStateChanged(int state);

public slots:
    void forceReadUpdate();

private slots:
    void readPendingDatagram();

private:
    struct Client;
    int addClientAndGetIndex(const Client& client);
    void sendDatagram(int clientIdx, const char *data, const qint64 size);
    quint8 getClientHash(int clientIdx);

private:
    friend void ::notifyz21getSystemInfo(uint8_t client);
    friend void notifyz21EthSend(uint8_t client, uint8_t *data);
    friend void notifyz21RailPower(uint8_t State);
    friend uint8_t notifyz21ClientHash(uint8_t client);
    friend void notifyz21LNdetector(uint8_t client, uint8_t typ, uint16_t Adr);

    friend class RBusRetroaction;
    friend class AccessoryManager;
    friend class LocoManager;

#ifdef WITH_LOCONET2
    friend class LocoNetZ21Adapter;
    friend class LoconetSlotServer;
#endif

    z21Class *m_z21 = nullptr;

private:
    QUdpSocket *m_udpServer = nullptr;
    QTimer *m_forceReadTimer = nullptr;
    bool socketReadScheduled = false;

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

    RBusRetroaction *m_RBUS = nullptr;
    AccessoryManager *m_accessoryMgr = nullptr;
    LocoManager *m_locoMgr = nullptr;

#ifdef WITH_LOCONET2
    LocoNetZ21Adapter *m_locoNetAdapter = nullptr;
#endif
};

#endif // Z21SERVER_H
