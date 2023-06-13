#include "loconetz21adapter.h"


#ifdef WITH_LOCONET2

#include "loconetz21source.h"
#include "loconettcpbinaryserver.h"
#include "loconetslotserver.h"

#include "server/z21server.h"
#include "server/loco/locomanager.h"
#include "server/accessories/accessorymanager.h"

#include "z21library/z21.h"
#include "z21library/z21header.h"

#include "loconetbus.h"

#include <QDebug>
#include <iostream>

void log_all_traffic(const LnMsg *msg)
{
    std::cout << std::flush;
    std::cerr << "LOCONET RX: " << fmtOpcode(msg->data[0]) << std::endl;
    std::cerr << std::flush;
}

LocoNetZ21Adapter::LocoNetZ21Adapter(Z21Server *server) :
    QObject(server),
    m_server(server)
{
    m_busHolder = new LocoNetBusHolder;
    m_source = new LocoNetZ21Source(&m_busHolder->bus, this);
    m_dispatcher = new LocoNetDispatcher(&m_busHolder->bus);
    m_slotServer = new LoconetSlotServer(m_busHolder, m_dispatcher, m_server, this);

    m_tcpServer = new LocoNetTCPBinaryServer(m_busHolder, this);
    m_tcpServer->startServer();

    auto powerStateHandler = [this](const LnMsg *msg)
    {
        Z21::PowerState curState = m_server->getPower();
        Z21::PowerState newState = curState;

        switch(msg->sz.command)
        {
        case OPC_GPON:
        {
            newState = Z21::PowerState::Normal;
            break;
        }
        case OPC_GPOFF:
        {
            if(curState != Z21::PowerState::Normal && curState != Z21::PowerState::EmergencyStop)
                break; //Leave untouched in this case
            newState = Z21::PowerState::TrackVoltageOff;
            break;
        }
        case OPC_IDLE:
            newState = Z21::PowerState::EmergencyStop;
            break;
        }

        if(newState == curState)
            return;

        m_server->setPower(newState);
    };

    //Register dispatcher callbacks
    m_dispatcher->onPacket(OPC_GPON, powerStateHandler);
    m_dispatcher->onPacket(OPC_GPOFF, powerStateHandler);
    m_dispatcher->onPacket(OPC_IDLE, powerStateHandler);

    m_dispatcher->onPacket(CALLBACK_FOR_ALL_OPCODES, &log_all_traffic);
    m_dispatcher->onSwitchRequest([this](uint16_t address, bool on, bool dir)
                                 {
                                     m_server->getAccessoryMgr()->setAccessoryState(address, dir);
                                 });
    m_dispatcher->onSwitchState([this](uint16_t address, bool on, bool dir)
                                  {
                                      m_server->getAccessoryMgr()->setAccessoryState(address, dir);
                                  });

    //Register Z21 callbacks
    connect(m_server, &Z21Server::powerStateChanged,
            this, &LocoNetZ21Adapter::setLocoNetPowerFromZ21);
    connect(m_server->getAccessoryMgr(), &AccessoryManager::accessoryStateChanged,
            this, [this](int idx, int port, bool val)
            {
                int address = idx * 8 + port;
                requestSwitch(&m_busHolder->bus, address, true, val);
            });

    std::cerr.flush();
    std::cout.flush();
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

void LocoNetZ21Adapter::setLocoNetPowerFromZ21(int state)
{
    Z21::PowerState powerState = Z21::PowerState(state);
    LnMsg packet;
    switch (powerState)
    {
    case Z21::PowerState::Normal:
        packet.data[0] = OPC_GPON;
        break;
    case Z21::PowerState::EmergencyStop:
        packet.data[0] = OPC_IDLE;
        break;
    default:
        packet.data[0] = OPC_GPOFF;
        break;
    }
    writeChecksum(packet);
    m_busHolder->bus.broadcast(packet, m_dispatcher);
}

LoconetSlotServer *LocoNetZ21Adapter::getSlotServer() const
{
    return m_slotServer;
}

#endif // WITH_LOCONET2
