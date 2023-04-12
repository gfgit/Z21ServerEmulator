#ifndef Z21SERVER_CONSTANTS_H
#define Z21SERVER_CONSTANTS_H

#include <QString>

namespace Z21 {

constexpr uint16_t defaultServerPort = 21105;

constexpr int Z21_UDP_TX_MAX_SIZE = 15; //--> POM DATA has 12 Byte!

//certain global XPressnet status indicators:
enum class PowerState : uint8_t
{
    Normal = 0x00,          // Normal Operation Resumed
    EmergencyStop = 0x01,   // Emergency Stop
    TrackVoltageOff = 0x02, // The track voltage is switched off
    ShortCircuit = 0x04,    // Short circuit
    ServiceMode = 0x08 	    // The programming mode is active - Service Mode
};

QString getPowerStateName(PowerState state); //Implemented in z21server.cpp

constexpr int S88_MODULE_COUNT = 62;
constexpr int ACCESSORY_COUNT = 1024;

} // namespace Z21

#endif // Z21SERVER_CONSTANTS_H
