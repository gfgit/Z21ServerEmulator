#ifndef LOCONETZ21ADAPTER_H
#define LOCONETZ21ADAPTER_H

#ifdef WITH_LOCONET2

#include <QObject>

class Z21Server;
class LocoNetZ21Source;
struct LocoNetBusHolder;
class LocoNetTCPBinaryServer;
class LoconetSlotServer;

class LocoNetDispatcher;

extern "C" void notifyz21LNSendPacket(uint8_t *data, uint8_t length);

class LocoNetZ21Adapter : public QObject
{
public:
    LocoNetZ21Adapter(Z21Server *server);
    ~LocoNetZ21Adapter();

    LoconetSlotServer *getSlotServer() const;

private:
    friend void notifyz21LNSendPacket(uint8_t *data, uint8_t length);
    void injectLNPacketFromZ21(uint8_t *data, uint8_t length);

    friend class LocoNetZ21Source;
    bool sendLNPacketToZ21(uint8_t *data, uint8_t length);

private slots:
    void setLocoNetPowerFromZ21(int state);

private:
    Z21Server *m_server;

    LocoNetBusHolder *m_busHolder;
    LocoNetZ21Source *m_source;
    LocoNetDispatcher *m_dispatcher;
    LocoNetTCPBinaryServer *m_tcpServer;
    LoconetSlotServer *m_slotServer;
};

#endif // LOCONETZ21ADAPTER_H

#endif // WITH_LOCONET2
