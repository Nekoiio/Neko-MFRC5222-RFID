#pragma once

#include<stdint.h>

enum class Command : uint8_t
{
    Idle = 0x00,
    Transceive = 0x0C,
    Authenticate = 0x0E,
    Reset = 0x0F,
    AntennaOn = 0x03,
    AntennaOff = 0x00

};