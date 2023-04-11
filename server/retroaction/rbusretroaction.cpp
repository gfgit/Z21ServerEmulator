#include "rbusretroaction.h"

#include "server/z21server.h"
#include "z21library/z21.h"

RBusRetroaction::RBusRetroaction(Z21Server *server) :
    QObject(server),
    m_server(server)
{

}

void RBusRetroaction::sendS88Status(int group)
{
    //return last state S88 Data for the Client!
    //Group of 10 modules

    int MAdr = 1;       //Feedback Group Module
    uint8_t datasend[11];   //Array group index (1 byte) & feedback status (10 bytes)
    datasend[0] = group; //requested group index

    //Iterate through all active modules in memory
    for (int m = (group * 10); m < Z21::S88_MODULE_COUNT; m++)
    {
        datasend[MAdr] = uint8_t(modules[m].to_ulong());
        MAdr++;         //Next module in the group
        if (MAdr >= 11) //10 modules of 8 ports each read
        {
            m_server->m_z21->setS88Data(datasend);
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
        m_server->m_z21->setS88Data(datasend);
    }
}

void RBusRetroaction::setInputState(int module, int port, bool val)
{
    //Each module has 8 ports
    if(module < 0 || module >= Z21::S88_MODULE_COUNT)
        return;

    bool oldVal = modules[module].test(port);
    if(oldVal == val)
        return;

    modules[module].set(port, val);
    emit inputStateChanged(module, port, val);

    int group = module / 10;
    sendS88Status(group);
}
