#ifndef LOCOMANAGER_H
#define LOCOMANAGER_H

#include <QObject>

#include "server/z21server_constants.h"

class Z21Server;

extern "C" void notifyz21LocoState(uint16_t Adr, uint8_t data[]);
extern "C" void notifyz21LocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt);


struct LocoSlot
{
    uint16_t adr;   // SS1, SS0, A13, A12| A11, A10, A9, A8| A7, A6, A5, A4| A3, A2, A1, A0
    // A0-A13 = Address
    // SS = Speed steps (0=error, 1=14, 2=28, 3=128)
    uint8_t speed;	//Dir, Speed 0..127 (0x00 - 0x7F) -> 0SSS SSSS + (0x80) -> D000 0000
    uint8_t f0;		//X   X   X   F0 | F4  F3  F2  F1
    uint8_t f1;		//F12 F11 F10 F9 | F8  F7  F6  F5
    uint8_t f2;		//F20 F19 F18 F17| F16 F15 F14 F13
    uint8_t f3;		//F28 F27 F26 F25| F24 F23 F22 F21

    inline uint16_t address() const
    {
        return adr & 0x3FFF;
    }

    inline void setSpeedSteps(Z21::DCCSpeedSteps steps)
    {
        adr = (adr & 0x3FFF) | (uint8_t(steps) << 14);
    }

    inline Z21::DCCSpeedSteps getSpeedSteps() const
    {
        return Z21::DCCSpeedSteps(adr >> 14);
    }

    inline bool getDirection() const
    {
        return (speed >> 7) & 0x01;
    }

    inline uint8_t getSpeed() const
    {
        return speed & 0x7F;
    }

    bool getFunction(int func) const;

    inline uint8_t getFunction0to4() const
    {
        return f0 & 0x1F; //gives function state - F0 F4 F3 F2 F1 back
    }

    inline uint8_t getFunction5to8() const
    {
        return f1 & 0x0F; //F8 F7 F6 F5
    }

    inline uint8_t getFunction9to12() const
    {
        return f1 >> 4; //F12 F11 F10 F9
    }

    inline uint8_t getFunction13to20() const
    {
        return f2; //F20 - F13
    }

    inline uint8_t getFunction21to28() const
    {
        return f3; //F28 - F21
    }

    inline uint8_t getFunction29to31() const
    {
        return f0 >> 5; //F31 - F29
    }



    inline void setFunctions0to4(uint8_t func)
    {
        f0 = (func & 0x1F) | (f0 & 0b11100000);
    }

    inline void setFunctions5to8(uint8_t func)
    {
        f1 = (f1 | 0x0F) & (func | 0xF0);
    }

    inline void setFunctions9to12(uint8_t func)
    {
        f1 = (f1 | 0xF0) & ((func << 4) | 0x0F);
    }

    inline void setFunctions13to20(uint8_t func)
    {
        f2 = func;
    }

    inline void setFunctions21to28(uint8_t func)
    {
        f3 = func;
    }

    inline void setFunctions29to36(uint8_t func)
    {
        f0 = (func << 5) | (f0 & 0x1F);
    }
};

class LocoManager : public QObject
{
    Q_OBJECT
public:
    explicit LocoManager(Z21Server *server);

    void setLocoFunction(uint16_t address, int func, bool val);
    bool getLocoFunction(uint16_t address, int func);

    bool setSpeed14(uint16_t address, uint8_t speed);
    bool setSpeed28(uint16_t address, uint8_t speed);
    bool setSpeed128(uint16_t address, uint8_t speed);

    inline uint8_t getFunction0to4(int address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getFunction0to4();
    }

    inline uint8_t getFunction5to8(int address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getFunction5to8();
    }

    inline uint8_t getFunction9to12(int address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getFunction9to12();
    }

    inline uint8_t getFunction13to20(int address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getFunction13to20();
    }

    inline uint8_t getFunction21to28(int address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getFunction21to28();
    }

    inline uint8_t getFunction29to31(int address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getFunction29to31();
    }

    void setFunctions0to4(uint16_t address, uint8_t func)
    {
        uint8_t Slot = getSlotForAddress(address);
        loco_slots[Slot].setFunctions0to4(func);
        emit locoSlotChanged(Slot);
        emit locoFuncChanged(address, 0); //First is enough to trigger batch
    }

    void setFunctions5to8(uint16_t address, uint8_t func)
    {
        uint8_t Slot = getSlotForAddress(address);
        loco_slots[Slot].setFunctions5to8(func);
        emit locoSlotChanged(Slot);
        emit locoFuncChanged(address, 5); //First is enough to trigger batch
    }

    void setFunctions9to12(uint16_t address, uint8_t func)
    {
        uint8_t Slot = getSlotForAddress(address);
        loco_slots[Slot].setFunctions9to12(func);
        emit locoSlotChanged(Slot);
        emit locoFuncChanged(address, 9); //First is enough to trigger batch
    }

    void setFunctions13to20(uint16_t address, uint8_t func)
    {
        uint8_t Slot = getSlotForAddress(address);
        loco_slots[Slot].setFunctions13to20(func);
        emit locoSlotChanged(Slot);
        emit locoFuncChanged(address, 13); //First is enough to trigger batch
    }

    void setFunctions21to28(uint16_t address, uint8_t func)
    {
        uint8_t Slot = getSlotForAddress(address);
        loco_slots[Slot].setFunctions21to28(func);
        emit locoSlotChanged(Slot);
        emit locoFuncChanged(address, 21); //First is enough to trigger batch
    }

    void setFunctions29to36(uint16_t address, uint8_t func)
    {
        uint8_t Slot = getSlotForAddress(address);
        loco_slots[Slot].setFunctions29to36(func);
        emit locoSlotChanged(Slot);
        emit locoFuncChanged(address, 29); //First is enough to trigger batch
    }

    bool getLocoDir(uint16_t address)
    {
        uint8_t Slot = getSlotForAddress(address);
        return loco_slots[Slot].getDirection();
    }

signals:
    void locoSlotChanged(int Slot);
    void locoSlotRequested(int address);
    void locoFuncChanged(int address, int func);

private:
    friend class LocoDriveModel;
    friend class LocoNetZ21Adapter;
    friend class LoconetSlotServer;
    friend void notifyz21LocoState(uint16_t Adr, uint8_t data[]);
    friend void notifyz21LocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt);

    void initSlot(uint8_t Slot, uint16_t address);
    uint8_t getSlotForAddress(uint16_t address);
    void getLocoData(uint8_t Slot, uint8_t loco_data[6]);
    void getLocoDataForAddress(uint16_t address, uint8_t loco_data[6]);
    void setLocoFuncHelper(uint16_t address, uint8_t type, uint8_t fkt);
    void updateZ21LocoState(uint16_t address);

private:
    Z21Server *m_server;

    LocoSlot loco_slots[Z21::MAX_LOCO_SLOTS];
    uint8_t slotFullNext = 0;
};

#endif // LOCOMANAGER_H
