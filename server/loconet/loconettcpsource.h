#ifndef LOCONETTCPSOURCE_H
#define LOCONETTCPSOURCE_H

#ifdef WITH_LOCONET2

#include "LocoNet2.h"

class LocoNetTCPBinaryServer;

class LocoNetTcpSource : public LocoNetConsumer
{
public:
    LocoNetTcpSource(LocoNetBus *bus, LocoNetTCPBinaryServer *tcpServer);
    ~LocoNetTcpSource();

    void sendRaw(const LnMsg &msg);

protected:
    LN_STATUS onMessage(const LnMsg &msg) override;

private:
    LocoNetBus *m_bus;
    LocoNetTCPBinaryServer *m_tcpServer;
};

#endif // WITH_LOCONET2

#endif // LOCONETTCPSOURCE_H
