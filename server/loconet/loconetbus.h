#ifndef LOCONETBUS_H
#define LOCONETBUS_H

#ifdef WITH_LOCONET2

#include "LocoNet2.h"

struct LocoNetBusHolder
{
    //This class is needed because LocoNetBus cannot be easily
    //forward declared in header
    LocoNetBusHolder() = default;

    LocoNetBus bus;
};

#endif // WITH_LOCONET2

#endif // LOCONETBUS_H
