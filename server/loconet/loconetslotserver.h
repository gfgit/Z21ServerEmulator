
#ifndef LOCONETSLOTSERVER_H
#define LOCONETSLOTSERVER_H

#ifdef WITH_LOCONET2

#include <QObject>
#include "server/z21server_constants.h"


typedef struct	//SLOT
{
    uint16_t LAdr;
    uint8_t Status;
    /* Slot Status 1
      D7-SL_SPURGE
        1=SLOT purge en,ALSO adrSEL (INTERNAL use only, not seen on NET!)
        CONDN/CONUP: bit encoding-Control double linked Consist List

    2 BITS for Consist
      D6-SL_CONUP
      D3-SL_CONDN
        11=LOGICAL MID CONSIST , Linked up AND down
        10=LOGICAL CONSIST TOP, Only linked downwards
        01=LOGICAL CONSIST SUB-MEMBER, Only linked upwards
        00=FREE locomotive, no CONSIST indirection/linking

    2 BITS for BUSY/ACTIVE
      D5-SL_BUSY
      D4-SL_ACTIVE
        11=IN_USE loco adr in SLOT -REFRESHED
        10=IDLE loco adr in SLOT, not refreshed
        01=COMMON loco adr IN SLOT, refreshed
        00=FREE SLOT, no valid DATA, not refreshed

    3 BITS for Decoder TYPE encoding for this SLOT
      D2-SL_SPDEX
      D1-SL_SPD14
      D0-SL_SPD28
        010=14 step MODE
        001=28 step. Generate Trinary packets for this Mobile ADR
        000=28 step/ 3 BYTE PKT regular mode
        011=128 speed mode packets
        111=128 Step decoder, Allow Advanced DCC consisting
        100=28 Step decoder ,Allow Advanced DCC consisting
   */
} LocoNetTypeSlot;

class LocoNetBusHolder;
class LocoNetDispatcher;
class Z21Server;

extern "C" uint8_t notifyz21LNdispatch(uint16_t Adr);


class LoconetSlotServer : public QObject
{
    Q_OBJECT
public:
    LoconetSlotServer(LocoNetBusHolder *bus,
                      LocoNetDispatcher *disp,
                      Z21Server *server,
                      QObject *parent = nullptr);

    void LNGetLocoStatus(uint8_t Slot);

    uint8_t LNGetSetLocoSlot(unsigned int Adr, bool add);

private:
    friend uint8_t notifyz21LNdispatch(uint16_t Adr);
    uint8_t LNdispatch(uint16_t Adr);
    void onLocoAdr(const uint8_t *msg_data);
    void onMoveSlot(const uint8_t *msg_data);
    void LN_OPC_SL_DATA(const uint8_t *LnPacketData);
    void onWRData(const uint8_t *msg_data);
    void onLocoSpeed(const uint8_t *LnPacketData);
    void sendLNDIRF(unsigned int Adr, uint8_t DIRF);
    void sendLNSPD(unsigned int Adr, uint8_t SPD);
    void sendLNSND(unsigned int Adr, uint8_t SND);
    void sendLNF3(unsigned int Adr, uint8_t F3);
    void sendLNF4(unsigned int Adr, uint8_t F4);
    void sendLNF5(unsigned int Adr, uint8_t F5);

private slots:
    void onLocoSlotRequested(int address);
    void onLocoFuncChanged(int address, int fkt);

private:
    LocoNetBusHolder *m_busHolder;
    LocoNetDispatcher *m_dispatcher;
    Z21Server *m_server;
    LocoNetTypeSlot m_slots[Z21::MAX_LOCO_SLOTS];
    uint8_t dispatchSlot = 0; //To put and store a SLOT for DISPATCHING
};

#endif // WITH_LOCONET2

#endif // LOCONETSLOTSERVER_H
