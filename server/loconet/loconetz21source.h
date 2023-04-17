#ifndef LOCONETZ21SOURCE_H
#define LOCONETZ21SOURCE_H

#ifdef WITH_LOCONET2

#include "LocoNet2.h"

class LocoNetZ21Adapter;

class LocoNetZ21Source : public LocoNetConsumer
{
public:
    LocoNetZ21Source(LocoNetBus *bus, LocoNetZ21Adapter *adapter);
    ~LocoNetZ21Source();

    void sendRaw(uint8_t *data, uint8_t length);

protected:
    LN_STATUS onMessage(const LnMsg &msg) override;

private:
    LocoNetBus *m_bus;
    LocoNetZ21Adapter *m_adapter;
};

#endif // WITH_LOCONET2

#endif // LOCONETZ21SOURCE_H
