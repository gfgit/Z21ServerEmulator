#include "arduinocompat.h"

#include <chrono>

#include <QUdpSocket>
#include <QDebug>

#include <iostream>
#include <bitset>

using ms = std::chrono::duration<double, std::milli>;

static std::chrono::system_clock::time_point last_sync;

uint64_t millis()
{
    ms elapsed = std::chrono::system_clock::now() - last_sync;
    return uint64_t(elapsed.count());
}

void init_millis()
{
    last_sync = std::chrono::system_clock::now();
}

////--------------------------------------------------------------------------------------------

ArduinoCompatEEPROM::ArduinoCompatEEPROM()
{
    //EEPROM bytes are default initialized to 255
    for(int i = 0; i < MAX_SIZE; i++)
        storage[i] = 255;
}

byte ArduinoCompatEEPROM::read(int address)
{
    if(address < 0 || address >= MAX_SIZE)
    {
        std::cerr << "EEPROM READ addr: " << address << " invalid, size is: " << MAX_SIZE << std::endl << std::flush;
        return 255;
    }

    byte value = storage[address];
    std::cerr << "EEPROM READ addr: " << address << " val: " << int(value) << std::endl << std::flush;
    return value;
}

void ArduinoCompatEEPROM::write(int address, byte value)
{
    if(address < 0 || address >= MAX_SIZE)
    {
        std::cerr << "EEPROM WRITE addr: " << address << " invalid, size is: " << MAX_SIZE << std::endl << std::flush;
        return;
    }

    std::cerr << "EEPROM WRITE addr: " << address << " val: " << int(value) << std::endl << std::flush;
    storage[address] = value;
}

////--------------------------------------------------------------------------------------------


void SerialEmulator::print(const char *str)
{
    if(str)
        std::cout << str << std::flush;
}

void SerialEmulator::print(int val, int base)
{
    auto flags = std::cout.flags();

    if(base == BIN)
    {
        std::bitset<sizeof(val) * 8> bit(val);
        std::cout << val;
    }
    else
    {
        switch (base)
        {
        case DEC:
            std::cout << std::dec;
            break;
        case HEX:
            std::cout << std::hex;
            break;
        default:
            break;
        }

        std::cout << val;
    }

    std::cout.flags(flags);
}

void SerialEmulator::println(const char *str)
{
    print(str);
    std::cout << std::endl << std::flush;
}

void SerialEmulator::println(int val, int base)
{
    print(val, base);
    std::cout << std::endl << std::flush;
}
