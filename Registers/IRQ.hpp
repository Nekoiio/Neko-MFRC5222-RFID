#pragma once

#include <stdint.h>

enum class IRQ : uint8_t
{
    Timer = 0x01,
    Error = 0x02,
    Rx = 0x20,
    Idle = 0x10
};