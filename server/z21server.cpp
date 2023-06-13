#include "z21server.h"

#include <QUdpSocket>
#include <QtEndian>

#include "z21library/z21.h"

#include "server/retroaction/rbusretroaction.h"
#include "server/accessories/accessorymanager.h"
#include "server/loco/locomanager.h"

#ifdef WITH_LOCONET2
#include "server/loconet/loconetz21adapter.h"
#include "server/loconet/loconetslotserver.h"
#endif

#include <thread>

#include <iostream> //console debugging

namespace Z21 {

//Defined in z21server_constants.h
QString getPowerStateName(PowerState state)
{
    switch (state)
    {
    case PowerState::Normal:
        return QLatin1String("Normal");
    case PowerState::EmergencyStop:
        return QLatin1String("Emergency Stop");
    case PowerState::TrackVoltageOff:
        return QLatin1String("Track Voltage Off");
    case PowerState::ShortCircuit:
        return QLatin1String("Short Circuit");
    case PowerState::ServiceMode:
        return QLatin1String("Service Mode");
    default:
        break;
    }

    return QString();
}

}

//Callbacks from Z21 Library

static Z21Server *m_instance = nullptr; //Used by callbacks

extern "C" void notifyz21getSystemInfo(uint8_t client)
{
    if(!m_instance)
        return;

    auto power = m_instance->getPower();

    //Report fake values
    uint16_t maincurrent = 15; //Ampere
    uint16_t mainvoltage = 15; //Volts
    uint16_t temp = 15; //Celsius degrees
    if(power == Z21::PowerState::TrackVoltageOff || power == Z21::PowerState::ShortCircuit)
    {
        maincurrent = 0;
        mainvoltage = 0;
    }

    m_instance->m_z21->sendSystemInfo(client, maincurrent, mainvoltage, temp);
}

extern "C" void notifyz21EthSend(uint8_t client, uint8_t *data)
{
    if(m_instance)
        m_instance->sendDatagram(client, reinterpret_cast<const char *>(data), data[0]);
}

extern "C" void notifyz21LNdetector(uint8_t client, uint8_t typ, uint16_t Adr)
{
    if(!m_instance)
        return;

    if (typ == 0x80)
    {
        //"Stationary Interrogate Request" (SIC)
        byte data[4];
        data[0] = 0x01; //Typ
        data[1] = Adr & 0xFF;
        data[2] = Adr >> 8;
        data[3] = m_instance->getAccessoryMgr()->getAccessoryState(Adr); //Condition of feedback

        m_instance->m_z21->setLNDetector(client, data, 4);
    }
}

extern uint8_t notifyz21LNdispatch(uint16_t Adr)
{
    //return the Slot that was dispatched, 0xFF at error!
    //TODO: error returns 0 instead of 0xFF
    if(!m_instance)
        return 0;

#ifdef WITH_LOCONET2
    return m_instance->getLocoNetAdapter()->getSlotServer()->LNdispatch(Adr);
#else
    return 0;
#endif
}

extern "C" void notifyz21LNSendPacket(uint8_t *data, uint8_t length)
{
    if(!m_instance)
        return;

#ifdef WITH_LOCONET2
    m_instance->getLocoNetAdapter()->injectLNPacketFromZ21(data, length);
#endif
}

//extern void notifyz21CANdetector(uint8_t client, uint8_t typ, uint16_t ID) __attribute__((weak));

extern "C" void notifyz21RailPower(uint8_t State)
{
    if(!m_instance)
        return;

    m_instance->setPower(Z21::PowerState(State));
}

/*
extern void notifyz21CVREAD(uint8_t cvAdrMSB, uint8_t cvAdrLSB) __attribute__((weak));
extern void notifyz21CVWRITE(uint8_t cvAdrMSB, uint8_t cvAdrLSB, uint8_t value) __attribute__((weak));
extern void notifyz21CVPOMWRITEBYTE(uint16_t Adr, uint16_t cvAdr, uint8_t value) __attribute__((weak));
extern void notifyz21CVPOMWRITEBIT(uint16_t Adr, uint16_t cvAdr, uint8_t value) __attribute__((weak));
extern void notifyz21CVPOMREADBYTE(uint16_t Adr, uint16_t cvAdr) __attribute__((weak));
extern void notifyz21CVPOMACCWRITEBYTE(uint16_t Adr, uint16_t cvAdr, uint8_t value) __attribute__((weak));
extern void notifyz21CVPOMACCWRITEBIT(uint16_t Adr, uint16_t cvAdr, uint8_t value) __attribute__((weak));
extern void notifyz21CVPOMACCREADBYTE(uint16_t Adr, uint16_t cvAdr) __attribute__((weak));
*/

extern "C" uint8_t notifyz21AccessoryInfo(uint16_t Adr)
{
    if(!m_instance)
        return 0;

    return m_instance->getAccessoryMgr()->getAccessoryState(Adr);
}

extern "C" void notifyz21Accessory(uint16_t Adr, bool state, bool active)
{
    Q_UNUSED(active) //TODO: What is the purpose of "active" flag?
    if(!m_instance)
        return;

    m_instance->getAccessoryMgr()->setAccessoryState(Adr, state);
}

//extern void notifyz21ExtAccessory(uint16_t Adr, byte state) __attribute__((weak));

extern "C" void notifyz21LocoState(uint16_t Adr, uint8_t data[])
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->getLocoDataForAddress(Adr, data);
}

extern "C" void notifyz21LocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setLocoFuncHelper(Adr, type, fkt);
}

extern "C" void notifyz21LocoFkt0to4(uint16_t Adr, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setFunctions0to4(Adr, fkt);
}

extern void notifyz21LocoFkt5to8(uint16_t Adr, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setFunctions5to8(Adr, fkt);
}

extern void notifyz21LocoFkt9to12(uint16_t Adr, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setFunctions9to12(Adr, fkt);
}

extern void notifyz21LocoFkt13to20(uint16_t Adr, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setFunctions13to20(Adr, fkt);
}

extern void notifyz21LocoFkt21to28(uint16_t Adr, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setFunctions21to28(Adr, fkt);
}

extern void notifyz21LocoFkt29to36(uint16_t Adr, uint8_t fkt)
{
    if(!m_instance)
        return;

    m_instance->getLocoMgr()->setFunctions29to36(Adr, fkt);
}

//extern void notifyz21LocoFkt37to44(uint16_t Adr, uint8_t fkt) __attribute__((weak));
//extern void notifyz21LocoFkt45to52(uint16_t Adr, uint8_t fkt) __attribute__((weak));
//extern void notifyz21LocoFkt53to60(uint16_t Adr, uint8_t fkt) __attribute__((weak));
//extern void notifyz21LocoFkt61to68(uint16_t Adr, uint8_t fkt) __attribute__((weak));
//extern void notifyz21LocoFktExt(uint16_t Adr, uint8_t low, uint8_t high) __attribute__((weak));

extern "C" void notifyz21LocoSpeed(uint16_t Adr, uint8_t speed, uint8_t steps)
{
    if(!m_instance)
        return;

    auto locoMgr = m_instance->getLocoMgr();

    switch (steps)
    {
    case 14: locoMgr->setSpeed14(Adr, speed); break;
    case 28: locoMgr->setSpeed28(Adr, speed); break;
    default: locoMgr->setSpeed128(Adr, speed);
    }
}

extern "C" void notifyz21S88Data(uint8_t group)
{
    if(!m_instance)
        return;

    m_instance->getRBUS()->sendS88Status(group);
}

//extern uint16_t notifyz21Railcom() __attribute__((weak));	//return global Railcom Adr

//extern void notifyz21UpdateConf() __attribute__((weak)); //information for DCC via EEPROM (RailCom, ProgMode,...)

extern "C" uint8_t notifyz21ClientHash(uint8_t client)
{
    if(!m_instance)
        return 0;
    return m_instance->getClientHash(client);
}


//---------------------------------------------------------
// Z21Server

Z21Server::Z21Server(QObject *parent) :
    QObject{parent}
{
    Q_ASSERT_X(!m_instance, "Z21Server", "Only one instance allowed");
    m_instance = this;

    m_z21 = new z21Class;
    m_RBUS = new RBusRetroaction(this);
    m_accessoryMgr = new AccessoryManager(this);
    m_locoMgr = new LocoManager(this);

#ifdef WITH_LOCONET2
    m_locoNetAdapter = new LocoNetZ21Adapter(this);
#endif

    m_udpServer = new QUdpSocket(this);
    connect(m_udpServer, &QUdpSocket::readyRead, this, &Z21Server::readPendingDatagram);
}

Z21Server::~Z21Server()
{
    delete m_z21;
    m_udpServer->close();

    m_instance = nullptr;
}

bool Z21Server::startServer(quint16 port)
{
    if(!m_udpServer->bind(port))
        return false;

    m_udpServer->setSocketOption(QUdpSocket::ReceiveBufferSizeSocketOption, 2 * 65536);
    qDebug() << "UDP RECV BUF:" << m_udpServer->socketOption(QUdpSocket::ReceiveBufferSizeSocketOption);

    setPower(Z21::PowerState::Normal);
    return true;
}

void Z21Server::setPower(Z21::PowerState state)
{
    if(state == getPower())
    {
        std::cerr << "Z21 POWER NOTIFY SAME: " << int(state) << std::endl << std::flush;
        return;
    }

    m_z21->setPower(byte(state));
    emit powerStateChanged(int(state));
}

Z21::PowerState Z21Server::getPower() const
{
    return Z21::PowerState(m_z21->getPower());
}

void Z21Server::readPendingDatagram()
{
    constexpr int MAX_BUF_SIZE = 4096;

    while(m_udpServer->hasPendingDatagrams())
    {
        //Allocate buffer
        qint64 sz = m_udpServer->pendingDatagramSize();
        if(sz > MAX_BUF_SIZE)
            sz = MAX_BUF_SIZE;

        std::unique_ptr<uint8_t[]> buf(new uint8_t[sz]);
        uint8_t *ptr = buf.get();

        //Receive datagram
        Client client;
        m_udpServer->readDatagram(reinterpret_cast<char *>(ptr), sz, &client.remoteAddr, &client.remotePort);

        //Register client
        int clientIdx = addClientAndGetIndex(client);

        //NOTE: a single datagram can contain multimple independent Z21 message
        while(sz > 2)
        {
            //Check message is fully read
            uint16_t msgSize = *reinterpret_cast<uint16_t *>(ptr);
            msgSize = qFromLittleEndian(msgSize);

            if(msgSize > sz)
                break;

            //Simulate network slowdown
            std::this_thread::sleep_for(std::chrono::milliseconds(250));

            //Handle message
            m_z21->receive(clientIdx, ptr);

            //Go to next message
            ptr += msgSize;
            sz -= msgSize;
        }
    }
}

int Z21Server::addClientAndGetIndex(const Client &client)
{
    //If client is already registered, get it's index
    int idx = m_clients.indexOf(client);
    if(idx == -1)
    {
        //Register new client
        idx = m_clients.size();
        m_clients.append(client);
    }

    //Reserve 0 for "broadcast" by adding 1 to index
    return idx + 1;
}

void Z21Server::sendDatagram(int clientIdx, const char *data, const qint64 size)
{
    if (clientIdx == 0)
    {
        //Broadcast message
        for(const Client& client : qAsConst(m_clients))
        {
            if(!client.remotePort || client.remoteAddr.isNull())
                continue; //Skip invalid clients

            m_udpServer->writeDatagram(data, size, client.remoteAddr, client.remotePort);
        }
    }
    else
    {
        const Client client = m_clients.value(clientIdx - 1);
        if(!client.remotePort || client.remoteAddr.isNull())
            return; //Skip invalid clients

        m_udpServer->writeDatagram(data, size, client.remoteAddr, client.remotePort);
    }
}

quint8 Z21Server::getClientHash(int clientIdx)
{
    const Client client = m_clients.value(clientIdx - 1);
    if(!client.remotePort || client.remoteAddr.isNull())
        return 0; //Skip invalid clients

    quint8 fourBytes[4] = {};
    *reinterpret_cast<quint32 *>(fourBytes) = client.remoteAddr.toIPv4Address();

    quint8 HashIP = fourBytes[0] ^ fourBytes[1] ^ fourBytes[2] ^ fourBytes[3]; //make Hash from IP
    return HashIP;
}

RBusRetroaction *Z21Server::getRBUS() const
{
    return m_RBUS;
}

AccessoryManager *Z21Server::getAccessoryMgr() const
{
    return m_accessoryMgr;
}

LocoManager *Z21Server::getLocoMgr() const
{
    return m_locoMgr;
}

#ifdef WITH_LOCONET2
LocoNetZ21Adapter *Z21Server::getLocoNetAdapter() const
{
    return m_locoNetAdapter;
}
#endif
