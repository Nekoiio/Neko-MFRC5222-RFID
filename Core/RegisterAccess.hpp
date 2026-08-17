#pragma once

#include <stdint.h>
#include <stddef.h>


#include "HAL/SPIBus.hpp"
#include "Registers/Registers.hpp"

class RegisterAccess
{
public:

    explicit RegisterAccess(SPIBus& spi);

    void writeRegister(Register reg, uint8_t value);

    uint8_t readRegister(Register reg);

    void readRegisters(
        Register reg,
        uint8_t* buffer,
        size_t length
    );

private:

    SPIBus& spi;

    uint8_t encodeWrite(Register reg);

    uint8_t encodeRead(Register reg);
};