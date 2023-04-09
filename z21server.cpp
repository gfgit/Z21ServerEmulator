#include "z21server.h"

#include <QUdpSocket>

#include "z21library/z21.h"


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

//extern void notifyz21LNdetector(uint8_t client, uint8_t typ, uint16_t Adr) __attribute__((weak));
//extern uint8_t notifyz21LNdispatch(uint16_t Adr) __attribute__((weak));
//extern void notifyz21LNSendPacket(uint8_t *data, uint8_t length) __attribute__((weak));

//extern void notifyz21CANdetector(uint8_t client, uint8_t typ, uint16_t ID) __attribute__((weak));

extern "C" void notifyz21RailPower(uint8_t State)
{
    if(!m_instance)
        return;

    emit m_instance->powerStateChanged(State);
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

extern uint8_t notifyz21AccessoryInfo(uint16_t Adr) __attribute__((weak));
extern void notifyz21Accessory(uint16_t Adr, bool state, bool active) __attribute__((weak));
extern void notifyz21ExtAccessory(uint16_t Adr, byte state) __attribute__((weak));

extern void notifyz21LocoState(uint16_t Adr, uint8_t data[]) __attribute__((weak));
extern void notifyz21LocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt0to4(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt5to8(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt9to12(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt13to20(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt21to28(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt29to36(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt37to44(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt45to52(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt53to60(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFkt61to68(uint16_t Adr, uint8_t fkt) __attribute__((weak));
extern void notifyz21LocoFktExt(uint16_t Adr, uint8_t low, uint8_t high) __attribute__((weak));
extern void notifyz21LocoSpeed(uint16_t Adr, uint8_t speed, uint8_t steps) __attribute__((weak));

extern void notifyz21S88Data(uint8_t group)
{
    if(!m_instance)
        return;

    m_instance->sendS88Status(group);
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
    m_server = new QUdpSocket(this);
    connect(m_server, &QUdpSocket::readyRead, this, &Z21Server::readPendingDatagram);
}

Z21Server::~Z21Server()
{
    delete m_z21;
    m_server->close();

    m_instance = nullptr;
}

bool Z21Server::startServer(quint16 port)
{
    if(!m_server->bind(port))
        return false;

    setPower(Z21::PowerState::Normal);
    return true;
}

void Z21Server::setPower(Z21::PowerState state)
{
    m_z21->setPower(byte(state));
    emit powerStateChanged(int(state));
}

Z21::PowerState Z21Server::getPower() const
{
    return Z21::PowerState(m_z21->getPower());
}

uint8_t Z21Server::getS88State(int module) const
{
    if(module >= Z21::S88_MODULE_COUNT)
        return 0;
    return S88ModuleState[module];
}

void Z21Server::setS88ModuleState(int module, uint8_t state)
{
    if(module >= Z21::S88_MODULE_COUNT)
        return;

    //Store value
    S88ModuleState[module] = state;

    //Modules are grouped by 10
    const int group = module / 10;

    //Update Z21
    sendS88Status(group);
}

void Z21Server::readPendingDatagram()
{
    while(m_server->hasPendingDatagrams())
    {
        //Receive datagram
        char buffer[Z21::Z21_UDP_TX_MAX_SIZE] = {};
        Client client;
        m_server->readDatagram(buffer, Z21::Z21_UDP_TX_MAX_SIZE, &client.remoteAddr, &client.remotePort);

        //Register client
        int clientIdx = addClientAndGetIndex(client);

        //Handle requests
        m_z21->receive(clientIdx, reinterpret_cast<uint8_t *>(buffer));
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

            m_server->writeDatagram(data, size, client.remoteAddr, client.remotePort);
        }
    }
    else
    {
        const Client client = m_clients.value(clientIdx - 1);
        if(!client.remotePort || client.remoteAddr.isNull())
            return; //Skip invalid clients

        m_server->writeDatagram(data, size, client.remoteAddr, client.remotePort);
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

void Z21Server::sendS88Status(int group)
{
    //return last state S88 Data for the Client!
    //Group of 10 modules

    byte MAdr = 1;       //Feedback Group Module
    byte datasend[11];   //Array group index (1 byte) & feedback status (10 bytes)
    datasend[0] = group; //requested group index

    //Iterate through all active modules in memory
    for (byte m = (group * 10); m < Z21::S88_MODULE_COUNT; m++)
    {
        datasend[MAdr] = S88ModuleState[m];
        MAdr++;         //Next module in the group
        if (MAdr >= 11) //10 modules of 8 ports each read
        {
            m_z21->setS88Data(datasend);
            return;
        }
    }

    if (MAdr < 11)
    {
        //Still unused modules available in the group? Set this 0x00 and then report!
        while (MAdr < 11)
        {
            datasend[MAdr] = 0x00; //fill last with zero
            MAdr++;                //Next module in the group
        }
        m_z21->setS88Data(datasend);
    }
}
