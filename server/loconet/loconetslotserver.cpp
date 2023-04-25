
#include "loconetslotserver.h"

#ifdef WITH_LOCONET2

#include "loconetbus.h"

#include "server/z21server.h"
#include "z21library/z21.h"

#include "server/loco/locomanager.h"

#define highByte(a) ( ( (a) >> 8 ) & 0xFF )
#define lowByte(b)  ( (b) & 0xFF )

//STAT1 => D2 = SL_SPDEX, D1 = SL_SPD14, D0 = SL_SPD28
#define LNLOCO14  0x02 //14 Speed Steps
#define LNLOCO28  0x01 //28 Speed Steps
#define LNLOCO128 0x07 //128 Speed Steps

//Define new OPC:
#ifndef OPC_UHLI_FUN
#define OPC_UHLI_FUN   0xD4  //Function 9-28 by Uhlenbrock
#endif

LoconetSlotServer::LoconetSlotServer(LocoNetBusHolder *bus,
                                     LocoNetDispatcher *disp,
                                     Z21Server *server,
                                     QObject *parent) :
    QObject{parent},
    m_busHolder(bus),
    m_dispatcher(disp),
    m_server(server)
{
    connect(m_server->getLocoMgr(), &LocoManager::locoSlotRequested,
            this, &LoconetSlotServer::onLocoSlotRequested);
    connect(m_server->getLocoMgr(), &LocoManager::locoFuncChanged,
            this, &LoconetSlotServer::onLocoFuncChanged);

    m_dispatcher->onPacket(OPC_RQ_SL_DATA, [this](const LnMsg *msg)
                           {
                               LNGetLocoStatus(msg->data[1]);
                           });
    m_dispatcher->onPacket(OPC_WR_SL_DATA, [this](const LnMsg *msg)
                           {
                               onWRData(msg->data);
                           });
    m_dispatcher->onPacket(OPC_LOCO_ADR, [this](const LnMsg *msg)
                           {
                               onLocoAdr(msg->data);
                           });
    m_dispatcher->onPacket(OPC_LOCO_SPD, [this](const LnMsg *msg)
                           {
                               onLocoSpeed(msg->data);
                           });
}

void LoconetSlotServer::LNGetLocoStatus(uint8_t Slot)
{
    //default Answer:
    uint8_t SLOT_DATA_READ[] = {OPC_SL_RD_DATA, 0x0E, Slot, 0, 0, 0, 0, 0x07, 0, 0, 0, 0, 0, 0};   //length = 14 = 0x0E
    if ((Slot != 0) && (Slot < Z21::MAX_LOCO_SLOTS))
    {    //wenn Lok im SLOT Server vorhanden ist --> update
        uint8_t ldata[6];
        m_server->getLocoMgr()->getLocoData(Slot, ldata);

#if defined(LnInvDir)
        byte DIRF = (((~ldata[1]) >> 2) & 0x20) | (ldata[2] & 0x1F);
#else
        uint8_t DIRF = ((ldata[1] >> 2) & 0x20) | (ldata[2] & 0x1F);
#endif

        SLOT_DATA_READ[3] = m_slots[Slot].Status;
        SLOT_DATA_READ[4] = lowByte(m_slots[Slot].LAdr & 0x7F);
        SLOT_DATA_READ[5] = lowByte(ldata[1] & 0x7F);
        SLOT_DATA_READ[6] = DIRF;                         //DIRF = 0,0,DIR,F0,F4,F3,F2,F1
        SLOT_DATA_READ[9] = lowByte((m_slots[Slot].LAdr >> 7) & 0x7F); //ADR2
        SLOT_DATA_READ[10] = lowByte(ldata[3] & 0x0F);    //SND = 0,0,0,0,F8,F7,F6,F5
    }
    //only report Lok Slots, no system slots!
    if (Slot < 120)
    {
        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(SLOT_DATA_READ);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
}

uint8_t LoconetSlotServer::LNGetSetLocoSlot(unsigned int Adr, bool add) {   //add command for MASTER-Mode only!
    if (Adr == 0)
        return 0;
    uint8_t getSlot = 0;
    for (uint8_t i = 1; i < Z21::MAX_LOCO_SLOTS; i++)
    {
        if (m_slots[i].LAdr == Adr)
            return i;  //already inside a SLOT
        if ((getSlot == 0) && (((m_slots[i].Status & 0x30) >> 4) == 0))
            getSlot = i;  //find a empty SLOT
    }
    if (getSlot != 0 && add == true)
    {    //add Adr to SLOT Server!
        m_slots[getSlot].Status = 0x10;   //ACTIVE - COMMON loco adr IN SLOT
#if defined(FS14)
        slot[getSlot].Status |= LNLOCO14;
#elif defined(FS28)
        slot[getSlot].Status |= LNLOCO28;
#else
        m_slots[getSlot].Status |= LNLOCO128;
#endif
        m_slots[getSlot].LAdr = Adr;

        //Report Slot DATA:
        LNGetLocoStatus(getSlot);

        return getSlot;
    }
    return 0;
}

void LoconetSlotServer::onLocoSlotRequested(int address)
{
    LNGetSetLocoSlot(address, false);
}

void LoconetSlotServer::onLocoFuncChanged(int address, int fkt)
{
    auto locoMgr = m_server->getLocoMgr();
    if (fkt <= 4)
    {
        uint8_t fkt = locoMgr->getFunction0to4(address);
        bool dir = locoMgr->getLocoDir(address);
        uint8_t DIRF = fkt | (dir << 5);
        sendLNDIRF(address,  DIRF);
    }
    else if (fkt >= 5 && fkt <= 8)
        sendLNSND(address, locoMgr->getFunction5to8(address));
    else if (fkt >= 9 && fkt <= 12)
        sendLNF3 (address, locoMgr->getFunction9to12(address));
    else if (fkt >= 13 && fkt <= 20)
        sendLNF4 (address, locoMgr->getFunction13to20(address));
    else if (fkt >= 21 && fkt <= 28)
        sendLNF5 (address, locoMgr->getFunction21to28(address));
}

//Check if Slot can be dispatched
uint8_t LoconetSlotServer::LNdispatch (uint16_t Adr)
{
    dispatchSlot = LNGetSetLocoSlot(Adr, true);  //add to SLOT
    if (((m_slots[dispatchSlot].Status & 0x30) >> 4) != 0b11)
    {
        //not 11=IN_USE
        return dispatchSlot;
    }
    dispatchSlot = 0;  //clear
    return 0;
}

void LoconetSlotServer::onLocoAdr(const uint8_t *msg_data)
{
    //0xBF = Request loco address

    //add to a SLOT:
    uint8_t newSlot = LNGetSetLocoSlot((msg_data[1] << 7) | (msg_data[2] & 0x7F), true); //ADR2:7 ms-bits = 0 bei kurzer Adr; ADR:7 ls-bit
    if (dispatchSlot != 0 && msg_data[1] == 0 && msg_data[2] == 0)
        newSlot = dispatchSlot;

    if (newSlot == 0) {
        //0xB4 = OPC_LONG_ACK No free slot available
        uint8_t Fail[] = {OPC_LONG_ACK, 0x3F, 0x00, 0x00}; //LACK,0 is returned[<B4>,<3F>,<0>,<CHK>]
        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(&Fail);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
    else {
        //Report Slot DATA:
        LNGetLocoStatus(newSlot);
    }
}

void LoconetSlotServer::onMoveSlot(const uint8_t *msg_data)
{
    //0xBA = Move slot SRC to DST
    if (msg_data[1] == 0) //SRC = 0
    {
        //SLOT READ DATA of DISPATCH Slot
        if (dispatchSlot != 0)
        {
            m_slots[dispatchSlot].Status = m_slots[dispatchSlot].Status | 0x30;  //IN_USE
            LNGetLocoStatus(dispatchSlot); //Give slot that was DISPATCHED
            dispatchSlot = 0;  //reset the Dispatch SLOT
            return;
        }
    }
    else if (msg_data[1] == msg_data[2])
    {  //NULL move
        //SRC=DEST is set to IN_USE , if legal move -> NULL move
        m_slots[msg_data[1]].Status = m_slots[msg_data[1]].Status | 0x30;  //B00011111;  //IN_USE
        LNGetLocoStatus(msg_data[1]);
        return;
    }
    else if (msg_data[2] == 0) //DST = 0
    {
        //DISPATCH Put, mark SLOT as DISPATCH;
        dispatchSlot = msg_data[1];
        //RETURN slot status <0xE7> of DESTINATION slot DEST if move legal
        LNGetLocoStatus(dispatchSlot);
        return;
    }

    //RETURN Fail LACK code if illegal move <B4>,<3A>,<0>,<chk>
    uint8_t Fail[] = {OPC_LONG_ACK, OPC_MOVE_SLOTS & 0x7F, 0x00, 0x00};
    LnMsg *answerMsg = reinterpret_cast<LnMsg *>(&Fail);
    writeChecksum(*answerMsg);
    m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
}

//OPC SL Data Auswerten
void LoconetSlotServer::LN_OPC_SL_DATA(const uint8_t *LnPacketData)
{
    if (LnPacketData[2] < Z21::MAX_LOCO_SLOTS)
    {
        m_slots[LnPacketData[2]].LAdr = (LnPacketData[9] << 7) | (LnPacketData[4] & 0x7F);    //ADR2 | ADR
        m_slots[LnPacketData[2]].Status = LnPacketData[3];  //Save new Status

#if defined(LnInvDir)
        uint8_t speed = (LnPacketData[5] & 0x7F) | (((~LnPacketData[6]) << 2) & 0x80); //DIR, S, S, S, S, S, S, S
#else
        uint8_t speed = (LnPacketData[5] & 0x7F) | ((LnPacketData[6] << 2) & 0x80); //DIR, S, S, S, S, S, S, S
#endif

        auto locoMgr = m_server->getLocoMgr();

        if ((m_slots[LnPacketData[2]].Status & 0x111) == LNLOCO14)
            locoMgr->setSpeed14(m_slots[LnPacketData[2]].LAdr, speed);  //DIRF & SPD
        else
        {
            if ((m_slots[LnPacketData[2]].Status & 0b111) == LNLOCO28)
                locoMgr->setSpeed28(m_slots[LnPacketData[2]].LAdr, speed);  //DIRF & SPD
            else
                locoMgr->setSpeed128(m_slots[LnPacketData[2]].LAdr, speed);  //DIRF & SPD
        }

        locoMgr->setFunctions0to4(m_slots[LnPacketData[2]].LAdr, LnPacketData[6] & 0x1F);  //DIRF = - F0 F4 F3 F2 F1
        locoMgr->setFunctions5to8(m_slots[LnPacketData[2]].LAdr, LnPacketData[10] & 0x0F);  //SND = - F8 F7 F6 F5
    }
}

void LoconetSlotServer::onWRData(const uint8_t *msg_data)
{
    //Write slot data
    uint8_t SLOT_RX = msg_data[2];
    if (SLOT_RX < Z21::MAX_LOCO_SLOTS)
    {
        /* Slot: 0  dispatch
         *       1 - 119   active locos     */


        LN_OPC_SL_DATA(msg_data); //send DCC message

        m_server->m_z21->setLocoStateExt(m_slots[SLOT_RX].LAdr);

#if defined(XPRESSNET)
        XpressNet.setSpeed(slot[SLOT_RX].LAdr, 128, (LnPacketData[5] & 0x7F) | ((LnPacketData[6] << 2) & 0x80));
        XpressNet.setFunc0to4(slot[SLOT_RX].LAdr, LnPacketData[6] & B00011111);
        XpressNet.setFunc5to8(slot[SLOT_RX].LAdr, LnPacketData[10] & 0x0F);
        XpressNet.ReqLocoBusy(slot[SLOT_RX].LAdr);   //Lok wird nicht von LokMaus gesteuert!
#endif
    }
    else
    {
        //OPC_WR_PT_DATA
        /*Slot: 120 - 127   reserved for System and Master control
         *      123   Fast Clock
         *      124   Programming Track
         *      127   Command Station Options */
        if (SLOT_RX == 0x7C)
        {
            //Write PT slot data.
            //NOT IMPLEMENTED
        }
        else if (SLOT_RX == 0x7B)
        {
            //Write Fast Clock slot data.
            //NOT IMPLEMENTED
        }
    }

    //Response:
    //0=busy/aborted, 1=accepted(OPC_SL_RD_DATA), 0×40=accepted blind(OPC_SL_RD_DATA), 0x7F=not implemented
    uint8_t ACK[] = {OPC_LONG_ACK, OPC_WR_SL_DATA & 0b01111111, 1, 0x00};
    LnMsg *answerMsg = reinterpret_cast<LnMsg *>(&ACK);
    writeChecksum(*answerMsg);
    m_busHolder->bus.broadcast(*answerMsg, m_dispatcher); //Send ACK

    LNGetLocoStatus(SLOT_RX);  //Send OPC_SL_RD_DATA
}

void LoconetSlotServer::onLocoSpeed(const uint8_t *LnPacketData)
{
    //0SSSSSS
    if (LnPacketData[1] < Z21::MAX_LOCO_SLOTS)
    {
        if (m_slots[LnPacketData[1]].LAdr == 0)
        {
            //Unknown!
            return;
        }

        auto locoMgr = m_server->getLocoMgr();

        int speed = LnPacketData[2];
        bool dir = locoMgr->getLocoDir(m_slots[LnPacketData[1]].LAdr);

        if ((m_slots[LnPacketData[1]].Status & 0b111) == LNLOCO14)
        {
            speed = speed / 128 * 14; //Remap to 14 steps
            locoMgr->setSpeed14(m_slots[LnPacketData[1]].LAdr, ((dir << 7) | speed));
        }
        else
        {
            if ((m_slots[LnPacketData[1]].Status & 0b111) == LNLOCO28)
            {
                speed = speed / 128 * 28; //Remap to 28 steps
                locoMgr->setSpeed28(m_slots[LnPacketData[1]].LAdr, ((dir << 7) | speed));
            }
            else
                locoMgr->setSpeed128(m_slots[LnPacketData[1]].LAdr, ((dir << 7) | speed));
        }

        m_server->m_z21->setLocoStateExt(m_slots[LnPacketData[1]].LAdr);

#if defined(XPRESSNET)
        XpressNet.setSpeed(slot[LnPacketData[1]].LAdr, 128, (dcc.getLocoDir(slot[LnPacketData[1]].LAdr) << 7) | LnPacketData[2]);
        XpressNet.ReqLocoBusy(slot[LnPacketData[1]].LAdr);   //Lok wird nicht von LokMaus gesteuert!
#endif
    }
}

//--------------------------------------------------------------------------------------------
//Set slot direction, function 0-4 state
// DIRF = 0,0,DIR,F0,F4,F3,F2,F1
void LoconetSlotServer::sendLNDIRF (unsigned int Adr, uint8_t DIRF)
{
    byte Slot = LNGetSetLocoSlot(Adr, true);
    if (Slot > 0) {
#if defined(LnInvDir)
        byte setDIRF[] = {OPC_LOCO_DIRF, Slot, lowByte( ((~DIRF) & 0x20) | (DIRF & 0x1F) ), 0x00};
#else
        byte setDIRF[] = {OPC_LOCO_DIRF, Slot, DIRF, 0x00};
#endif
        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(setDIRF);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
}
//--------------------------------------------------------------------------------------------
//Set slot speed and update dir
void LoconetSlotServer::sendLNSPD (unsigned int Adr, uint8_t SPD)
{
    byte Slot = LNGetSetLocoSlot(Adr, true);
    if (Slot == 0)
        return;

    auto locoMgr = m_server->getLocoMgr();

    //Check if direction will change also?
    if (locoMgr->getLocoDir(Adr) != (SPD >> 7))
    {
        //calculate new direction:
        byte DIRF = locoMgr->getFunction0to4(Adr) | ((SPD >> 2) & 0x20);
        sendLNDIRF(Adr, DIRF);
    }

    byte setSPD[] = {OPC_LOCO_SPD, Slot, 0x7F, 0x00};
    setSPD[2] &= SPD;

    LnMsg *answerMsg = reinterpret_cast<LnMsg *>(setSPD);
    writeChecksum(*answerMsg);
    m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
}
//--------------------------------------------------------------------------------------------
//Set slot second function
// SND = 0,0,0,0,F8,F7,F6,F5
void LoconetSlotServer::sendLNSND (unsigned int Adr, uint8_t SND)
{
    byte Slot = LNGetSetLocoSlot(Adr, true);
    if (Slot > 0)
    {
        byte setSND[] = {OPC_LOCO_SND, Slot, SND, 0x00};

        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(setSND);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
}

//--------------------------------------------------------------------------------------------
// F3 = 0,0,0,0,F12,F11,F10,F9
void LoconetSlotServer::sendLNF3 (unsigned int Adr, uint8_t F3)
{
    byte Slot = LNGetSetLocoSlot(Adr, true);
    if (Slot > 0)
    {
        byte setF3[] = {0xA3, Slot, F3, 0x00};

        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(setF3);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
}

//--------------------------------------------------------------------------------------------
// F4 = F20,F19,F18,F17,F16,F15,F14,F13
void LoconetSlotServer::sendLNF4 (unsigned int Adr, uint8_t F4)
{
    byte Slot = LNGetSetLocoSlot(Adr, true);
    if (Slot > 0)
    {
        byte setF4[] = {OPC_UHLI_FUN, 0x20, Slot, 0x08, 0x7F, 0x00}; // - F19-F13
        setF4[4] &= F4;

        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(setF4);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);

        byte F2028 = ((m_server->getLocoMgr()->getFunction21to28(Adr) & 0x80) >> 1) | ((F4 & 0x80) >> 2);  // - f28, f20, ---
        byte setF2028[] = {OPC_UHLI_FUN, 0x20, Slot, 0x05, F2028, 0x00};  // - F28,F20 ----

        answerMsg = reinterpret_cast<LnMsg *>(setF2028);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
}
//--------------------------------------------------------------------------------------------
// F5 = F28,F27,F26,F25,F24,F23,F22,F21
void LoconetSlotServer::sendLNF5 (unsigned int Adr, uint8_t F5)
{
    byte Slot = LNGetSetLocoSlot(Adr, true);
    if (Slot > 0)
    {
        byte setF5[] = {OPC_UHLI_FUN, 0x20, Slot, 0x09, 0x7F, 0x00}; // - F27-F21
        setF5[4] &= F5;

        LnMsg *answerMsg = reinterpret_cast<LnMsg *>(setF5);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);

        byte F2028 = ((F5 & 0x80) >> 1) | ((m_server->getLocoMgr()->getFunction13to20(Adr) & 0x80) >> 2);  // - f28, f20, ---
        byte setF2028[] = {OPC_UHLI_FUN, 0x20, Slot, 0x05, F2028, 0x00};  // - F28,F20 ----

        answerMsg = reinterpret_cast<LnMsg *>(setF2028);
        writeChecksum(*answerMsg);
        m_busHolder->bus.broadcast(*answerMsg, m_dispatcher);
    }
}

#endif // WITH_LOCONET2
