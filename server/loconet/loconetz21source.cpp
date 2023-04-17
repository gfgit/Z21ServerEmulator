#include "loconetz21source.h"
#include "loconetz21adapter.h"

#ifdef WITH_LOCONET2

LocoNetZ21Source::LocoNetZ21Source(LocoNetBus *bus, LocoNetZ21Adapter *adapter) :
    m_bus(bus),
    m_adapter(adapter)
{
    m_bus->addConsumer(this);
}

LocoNetZ21Source::~LocoNetZ21Source()
{
    m_bus->removeConsumer(this);
    m_bus = nullptr;
}

void LocoNetZ21Source::sendRaw(uint8_t *data, uint8_t length)
{
    LnMsg SendPacket;

    //copy everything without XOR
    for (int i = 0; i < (length - 1); i++)
    {
        SendPacket.data[i] = *data; //prepare lnMsg to send
        data++;
    }

    writeChecksum(SendPacket);
    m_bus->broadcast(SendPacket, this);
}

LN_STATUS LocoNetZ21Source::onMessage(const LnMsg &msg)
{
    LnMsg copy = msg;
    if(!m_adapter->sendLNPacketToZ21(copy.data, copy.length()))
        return LN_STATUS::LN_UNKNOWN_ERROR;
    return LN_STATUS::LN_DONE;
}

#endif // WITH_LOCONET2
