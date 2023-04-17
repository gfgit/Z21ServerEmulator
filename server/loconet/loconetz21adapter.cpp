#include "loconetz21adapter.h"


#ifdef WITH_LOCONET2

#include "loconetz21source.h"

#include "server/z21server.h"
#include "z21library/z21.h"
#include "z21library/z21header.h"

struct LocoNetBusHolder
{
    //This class is needed because LocoNetBus cannot be easily
    //forward declared in header
    LocoNetBusHolder() = default;

    LocoNetBus bus;
};


LocoNetZ21Adapter::LocoNetZ21Adapter(Z21Server *server) :
    QObject(server),
    m_server(server)
{
    m_busHolder = new LocoNetBusHolder;
    m_source = new LocoNetZ21Source(&m_busHolder->bus, this);
    m_dispatcher = new LocoNetDispatcher(&m_busHolder->bus);

    //Register dispatcher callbacks
    m_dispatcher->onPowerChange(std::bind(&LocoNetZ21Adapter::setZ21PowerFromLocoNet,
                                          this, std::placeholders::_1));

    //Register Z21 callbacks
    connect(m_server, &Z21Server::powerStateChanged,
            this, &LocoNetZ21Adapter::setLocoNetPowerFromZ21);
}

LocoNetZ21Adapter::~LocoNetZ21Adapter()
{
    m_busHolder->bus.removeConsumer(m_dispatcher);
    delete m_dispatcher;
    m_dispatcher = nullptr;

    delete m_source;
    m_source = nullptr;

    delete m_busHolder;
    m_busHolder = nullptr;
}

void LocoNetZ21Adapter::injectLNPacketFromZ21(uint8_t *data, uint8_t length)
{
    m_source->sendRaw(data, length);
}

bool LocoNetZ21Adapter::sendLNPacketToZ21(uint8_t *data, uint8_t length)
{
    return m_server->m_z21->setLNMessage(data, length, Z21bcLocoNet_s, true);
}

void LocoNetZ21Adapter::setZ21PowerFromLocoNet(bool on)
{
    Z21::PowerState curState = m_server->getPower();
    if(!on && curState != Z21::PowerState::Normal)
    {
        /* NOTE: prevent infinite loop
        This is caused by the lack of different "OFF" states in LocoNet
        If Z21 sets a state different from Z21::PowerState::Normal
        here we would represent it as "OFF" -> Z21::PowerState::TrackVoltageOff
        but we should keep Z21 state unchanged
        */
        return; //Keep current status which is already "OFF"
    }

    Z21::PowerState newState = on ? Z21::PowerState::Normal : Z21::PowerState::TrackVoltageOff;
    if(newState == curState)
        return;

    m_server->setPower(newState);
}

void LocoNetZ21Adapter::setLocoNetPowerFromZ21(int state)
{
    Z21::PowerState powerState = Z21::PowerState(state);
    bool on = powerState == Z21::PowerState::Normal;
    reportPower(&m_busHolder->bus, on);
}

#endif // WITH_LOCONET2
