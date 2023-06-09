#include "locomanager.h"

#include "server/z21server.h"
#include "z21library/z21.h"

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


LocoManager::LocoManager(Z21Server *server) :
    QObject{server},
    m_server(server)
{

}

void LocoManager::setLocoFunction(uint16_t address, int func, bool val)
{
    setLocoFuncHelper(address, func, val);
    updateZ21LocoState(address);
}

bool LocoManager::getLocoFunction(uint16_t address, int func)
{
    uint8_t Slot = getSlotForAddress(address);
    return loco_slots[Slot].getFunction(func);
}

bool LocoManager::setSpeed14(uint16_t address, uint8_t speed)
{
    if (address == 0)
    {
        // Invalid address
        return false;
    }

    uint8_t Slot = getSlotForAddress(address);
    loco_slots[Slot].speed = speed;
    loco_slots[Slot].setSpeedSteps(Z21::DCCSpeedSteps::_14);
    emit locoSlotChanged(Slot);

    return true;
}

bool LocoManager::setSpeed28(uint16_t address, uint8_t speed)
{
    if (address == 0)
    {
        // Invalid address
        return false;
    }

    uint8_t Slot = getSlotForAddress(address);
    loco_slots[Slot].speed = speed;
    loco_slots[Slot].setSpeedSteps(Z21::DCCSpeedSteps::_28);
    emit locoSlotChanged(Slot);

    return true;
}

bool LocoManager::setSpeed128(uint16_t address, uint8_t speed)
{
    if (address == 0)
    {
        // Invalid address
        return false;
    }

    uint8_t Slot = getSlotForAddress(address);
    loco_slots[Slot].speed = speed;
    loco_slots[Slot].setSpeedSteps(Z21::DCCSpeedSteps::_128);
    emit locoSlotChanged(Slot);

    return true;
}

void LocoManager::initSlot(uint8_t Slot, uint16_t address)
{
    const Z21::DCCSpeedSteps DCCdefaultSteps = Z21::DCCSpeedSteps::_128;
    loco_slots[Slot].adr = address | (uint8_t(DCCdefaultSteps) << 14);
    //0x4000; //0xC000;	// c = '3' => 128 speed steps
    loco_slots[Slot].speed = 0x80;	//default direction is forward
    loco_slots[Slot].f0 = 0x00;
    loco_slots[Slot].f1 = 0x00;
    loco_slots[Slot].f2 = 0x00;
    loco_slots[Slot].f3 = 0x00;
    emit locoSlotChanged(Slot);
}

uint8_t LocoManager::getSlotForAddress(uint16_t address)
{
    uint8_t Slot;
    for (Slot = 0; Slot < Z21::MAX_LOCO_SLOTS; Slot++)
    {
        if ((loco_slots[Slot].adr & 0x3FFF) == address)
            return Slot; //Found locomotive
        if ((loco_slots[Slot].adr & 0x3FFF) == 0) {
            //Empty? New free slot
            initSlot(Slot, address); //Register
            return Slot;
        }
    }

    //no slot available!
    //start at the beginning with the overwriting
    //of existing slots
    Slot = slotFullNext;
    initSlot(Slot, address); //clear Slot!
    slotFullNext++;
    if (slotFullNext >= Z21::MAX_LOCO_SLOTS)
        slotFullNext = 0;
    return Slot;
}

void LocoManager::getLocoData(uint16_t address, uint8_t loco_data[])
{
    //uint8_t Steps, uint8_t Speed, uint8_t F0, uint8_t F1, uint8_t F2, uint8_t F3		==> F0 bis F31
    uint8_t Slot = getSlotForAddress(address);
    loco_data[0] = loco_slots[Slot].adr >> 14; 	//Steps
    loco_data[1] = loco_slots[Slot].speed;
    loco_data[2] = loco_slots[Slot].f0;	//F31 F30 F29 F0 - F4 F3 F2 F1
    loco_data[3] = loco_slots[Slot].f1;	//F12 - F5
    loco_data[4] = loco_slots[Slot].f2;	//F20 - F13
    loco_data[5] = loco_slots[Slot].f3;	//F28 - F21
}

void LocoManager::setLocoFuncHelper(uint16_t address, uint8_t type, uint8_t fkt)
{
    //type => 0 = OFF; 1 = ON; 2 = TOGGLE; 3 = error
    bool fktbit = 0; //New FKT bit to change
    if (type == 1)   //ON
        fktbit = 1;

    uint8_t Slot = getSlotForAddress(address);

    //Determine and reset the bit to change:
    if (fkt <= 4)
    {
        uint8_t func = loco_slots[Slot].f0 & 0x1F; //last state of functions 000 F0 F4.. Q1
        if (type == 2) {                           //toggle
            if (fkt == 0)
                fktbit = !(bitRead(func, 4));
            else
                fktbit = !(bitRead(func, fkt - 1));
        }
        if (fkt == 0)
            bitWrite(func, 4, fktbit);
        else
            bitWrite(func, fkt - 1, fktbit);
        //Daten senden:
        loco_slots[Slot].setFunctions0to4(func); //func = 0 0 0 F0 F4 F3 F2 F1
    }
    else if ((fkt >= 5) && (fkt <= 8))
    {
        uint8_t funcG2 = loco_slots[Slot].f1 & 0x0F; //letztes Zustand der Funktionen 0000 F8..F5
        if (type == 2)                               //um
            fktbit = !(bitRead(funcG2, fkt - 5));
        bitWrite(funcG2, fkt - 5, fktbit);
        //Daten senden:
        loco_slots[Slot].setFunctions5to8(funcG2); //funcG2 = 0 0 0 0 F8 F7 F6 F5
    }
    else if ((fkt >= 9) && (fkt <= 12))
    {
        uint8_t funcG3 = loco_slots[Slot].f1 >> 4; //letztes Zustand der Funktionen 0000 F12..F9
        if (type == 2)                             //um
            fktbit = !(bitRead(funcG3, fkt - 9));
        bitWrite(funcG3, fkt - 9, fktbit);
        //Daten senden:
        loco_slots[Slot].setFunctions9to12(funcG3); //funcG3 = 0 0 0 0 F12 F11 F10 F9
    }
    else if ((fkt >= 13) && (fkt <= 20))
    {
        uint8_t funcG4 = loco_slots[Slot].f2;
        if (type == 2) //toggle
            fktbit = !(bitRead(funcG4, fkt - 13));
        bitWrite(funcG4, fkt - 13, fktbit);
        //Daten senden:
        loco_slots[Slot].setFunctions13to20(funcG4); //funcG4 = F20 F19 F18 F17 F16 F15 F14 F13
    }
    else if ((fkt >= 21) && (fkt <= 28))
    {
        uint8_t funcG5 = loco_slots[Slot].f3;
        if (type == 2) //um
            fktbit = !(bitRead(funcG5, fkt - 21));
        bitWrite(funcG5, fkt - 21, fktbit);
        //Daten senden:
        loco_slots[Slot].setFunctions21to28(funcG5); //funcG5 = F28 F27 F26 F25 F24 F23 F22 F21
    }
    else if ((fkt >= 29) && (fkt <= 36))
    {
        uint8_t func = loco_slots[Slot].f0 >> 5;
        if (type == 2) //um
            fktbit = !(bitRead(func, fkt - 29));
        bitWrite(func, fkt - 29, fktbit);
        //Daten senden:
        loco_slots[Slot].setFunctions29to36(func);
    }

    emit locoSlotChanged(Slot);
}

void LocoManager::updateZ21LocoState(uint16_t address)
{
    m_server->m_z21->setLocoStateExt(address);
}

bool LocoSlot::getFunction(int func) const
{
    if (func <= 4)
    {
        uint8_t val = getFunction0to4();
        if(func == 0)
            return bitRead(val, 4);
        return bitRead(val, func - 1);
    }
    else if ((func >= 5) && (func <= 8))
    {
        uint8_t val = getFunction5to8();
        return bitRead(val, func - 5);
    }
    else if ((func >= 9) && (func <= 12))
    {
        uint8_t val = getFunction9to12();
        return bitRead(val, func - 9);
    }
    else if ((func >= 13) && (func <= 20))
    {
        uint8_t val = getFunktion13to20();
        return bitRead(val, func - 13);
    }
    else if ((func >= 21) && (func <= 28))
    {
        uint8_t val = getFunktion21to28();
        return bitRead(val, func - 21);
    }
    else if ((func >= 29) && (func <= 36))
    {
        uint8_t val = getFunktion29to31();
        return bitRead(val, func - 29);
    }

    return false;
}
