#include "loconettcpsource.h"

#ifdef WITH_LOCONET2

#include "loconettcpbinaryserver.h"

LocoNetTcpSource::LocoNetTcpSource(LocoNetBus *bus, LocoNetTCPBinaryServer *tcpServer) :
    m_bus(bus),
    m_tcpServer(tcpServer)
{
    m_bus->addConsumer(this);
}

LocoNetTcpSource::~LocoNetTcpSource()
{
    m_bus->removeConsumer(this);
    m_bus = nullptr;
}

void LocoNetTcpSource::sendRaw(const LnMsg &msg)
{
    LnMsg SendPacket = msg;
    m_bus->broadcast(SendPacket, this);
}

LN_STATUS LocoNetTcpSource::onMessage(const LnMsg &msg)
{
    LnMsg copy = msg;
    if(!m_tcpServer->broadcastMessage(copy.data, copy.length()))
        return LN_STATUS::LN_UNKNOWN_ERROR;
    return LN_STATUS::LN_DONE;
}

#endif // WITH_LOCONET2
