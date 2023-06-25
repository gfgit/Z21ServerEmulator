#ifndef Z21SERVER_CONSTANTS_H
#define Z21SERVER_CONSTANTS_H

#include <QString>

namespace Z21 {

constexpr uint16_t defaultServerPort = 21105;

constexpr int Z21_UDP_TX_MAX_SIZE = 15; //--> POM DATA has 12 Byte!

// certain global XPressnet status indicators:
enum class PowerState : uint8_t {
    Normal = 0x00,          // Normal Operation Resumed
    EmergencyStop = 0x01,   // Emergency Stop
    TrackVoltageOff = 0x02, // The track voltage is switched off
    ShortCircuit = 0x04,    // Short circuit
    ServiceMode = 0x08      // The programming mode is active - Service Mode
};

QString getPowerStateName(PowerState state); // Implemented in z21server.cpp

constexpr int S88_MODULE_COUNT = 62;
constexpr int ACCESSORY_COUNT = 1024;
constexpr int MAX_LOCO_SLOTS = 15;

// DCC Speed Steps
enum class DCCSpeedSteps {
    _14 = 0x01,
    _28 = 0x02,
    _128 = 0x03
};

inline int speedStepsToInt(Z21::DCCSpeedSteps steps)
{
    switch (steps)
    {
    case Z21::DCCSpeedSteps::_14:
        return 14;
    case Z21::DCCSpeedSteps::_28:
        return 28;
    case Z21::DCCSpeedSteps::_128:
        return 128;
    default:
        break;
    }
    return 0;
}

inline Z21::DCCSpeedSteps speedStepsToEnum(int steps)
{
    switch (steps)
    {
    case 14:
        return Z21::DCCSpeedSteps::_14;
    case 28:
        return Z21::DCCSpeedSteps::_28;
    default:
    case 128:
        return Z21::DCCSpeedSteps::_128;
    }
}

} // namespace Z21

#endif // Z21SERVER_CONSTANTS_H
