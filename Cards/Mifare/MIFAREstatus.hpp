#pragma once

#include <stdint.h>

enum class MIFAREStatus : uint8_t
{
    Ok              = 0x00,
    Error           = 0x01,
    Collision       = 0x02,
    Timeout         = 0x03,
    NoRoom          = 0x04,
    InternalError   = 0x05,
    Invalid         = 0x06,
    CRCWrong        = 0x07,
    ProtocolError    = 0x08,
    MifareNack      = 0xFF
};