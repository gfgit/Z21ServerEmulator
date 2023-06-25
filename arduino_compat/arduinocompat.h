#ifndef ARDUINOCOMPAT_H
#define ARDUINOCOMPAT_H

#include <cstdint>

// Arduino compatibility types
typedef uint8_t byte;
typedef bool boolean;

#define word(a, b)                     (((a)&0xFF) << 8) | ((b)&0xFF)
#define highByte(a)                    (((a) >> 8) & 0xFF)
#define lowByte(b)                     ((b)&0xFF)

#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

uint64_t millis();
void init_millis();

// void arduino_setup();
// void arduino_loop();

// Debugging
// #define SERIALDEBUG

enum SerialEmuBase {
    DEC = 0,
    BIN,
    HEX
};

class SerialEmulator
{
public:
    SerialEmulator() = default;

    void print(const char *str = nullptr);
    void print(int val, int base = DEC);
    void println(const char *str = nullptr);
    void println(int val, int base = DEC);
};

static SerialEmulator Serial;

class ArduinoCompatEEPROM
{
public:
    ArduinoCompatEEPROM();

    byte read(int address);

    void write(int address, byte value);

    void update(int address, byte value)
    {
        write(address, value);
    }

    static const int MAX_SIZE = 32 * 1024; // 32 KB storage
    byte storage[MAX_SIZE];
};

static ArduinoCompatEEPROM EEPROM;

#endif // ARDUINOCOMPAT_H
