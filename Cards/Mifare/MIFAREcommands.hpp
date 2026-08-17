#pragma once

#include <stdint.h>



enum class MIFARECommand : uint8_t
{
    AuthenticateA   = 0x60,
    AuthenticateB   = 0x61,
    Read            = 0x30,
    Write           = 0xA0,
    Decrement       = 0xC0,
    Increment       = 0xC1,
    Restore         = 0xC2,
    Transfer        = 0xB0,
    REQA            = 0x26,
    WUPA            = 0x52
};