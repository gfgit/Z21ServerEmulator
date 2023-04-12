#include "accessorymanager.h"

#include "server/z21server.h"
#include "z21library/z21.h"

extern "C" void notifyz21LNdetector(uint8_t client, uint8_t typ, uint16_t Adr);

AccessoryManager::AccessoryManager(Z21Server *server) :
    QObject(server),
    m_server(server)
{

}

void AccessoryManager::setAccessoryState(int index, int port, bool val)
{
    //Each accessory has 8 ports
    if(index < 0 || index >= Z21::ACCESSORY_COUNT / 8)
        return;

    //bool oldVal = accessories[index].test(port);
    //if(oldVal == val)
    //    return;

    accessories[index].set(port, val);
    emit accessoryStateChanged(index, port, val);

    //int group = module / 10;
    //sendS88Status(group);

    const uint16_t address = index * 8 + port;
    m_server->m_z21->setTrntInfo(address, val);

    //HACK: notify from LocoNet
    //notifyz21LNdetector(0, 0x80, address);
}
