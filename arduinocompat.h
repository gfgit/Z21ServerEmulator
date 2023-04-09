#ifndef ARDUINOCOMPAT_H
#define ARDUINOCOMPAT_H

#include <cstdint>

//Arduino compatibility types
typedef uint8_t byte;
typedef bool boolean;

#define word(a, b)  ( ( (a) & 0xFF ) << 8 ) | ( (b) & 0xFF )
#define highByte(a) ( ( (a) >> 8 ) & 0xFF )
#define lowByte(b)  ( (b) & 0xFF )

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

uint64_t millis();
void init_millis();

void arduino_setup();
void arduino_loop();


class ArduinoCompatEEPROM
{
public:
    ArduinoCompatEEPROM() = default;

    byte read(int address)
    {
        (void)address;
        return 0;
    }

    void write(int address, byte value)
    {
        (void)address;
        (void)value;
    }

    void update(int address, byte value)
    {
        write(address, value);
    }
};

static ArduinoCompatEEPROM EEPROM;

#endif // ARDUINOCOMPAT_H
