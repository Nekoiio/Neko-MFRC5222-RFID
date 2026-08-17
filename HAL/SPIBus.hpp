#pragma once

#include <stdint.h>
#include <stddef.h>

class SPIBus
{
public:

    virtual void begin() = 0;

    virtual void beginTransaction() = 0;

    virtual void endTransaction() = 0;

    // Transfer one byte
    virtual uint8_t transfer(uint8_t byte) = 0;

    // Transfer an entire SPI packet
    virtual void transfer(uint8_t* buffer, size_t length) = 0;

    virtual ~SPIBus() = default;
};