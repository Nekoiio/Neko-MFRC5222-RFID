#pragma once

#include <stdint.h>

enum class Register : uint8_t
{
    Command     = 0x01,
    ComIEn      = 0x02,
    DivIEn      = 0x03,
    ComIrq      = 0x04,

    Error       = 0x06,

    Status1     = 0x07,
    Status2     = 0x08,

    FIFOData    = 0x09,
    FIFOLevel   = 0x0A,

    Control     = 0x0C,

    BitFraming  = 0x0D,

    Mode        = 0x11,

    TxControl   = 0x14,

    TxASK       = 0x15,

    TxSel       = 0x16,

    RxSel       = 0x17,

    RxThreshold = 0x18,

    Demod       = 0x19,

    MfTx        = 0x1C,
    MfRx        = 0x1D,

    SerialSpeed = 0x1F,

    CRCResultH  = 0x21,
    CRCResultL  = 0x22,

    ModWidth    = 0x24,

    RFCfg       = 0x26,

    GsN         = 0x27,

    CWGsP       = 0x28,

    ModGsP      = 0x29,

    TMode       = 0x2A,
    TPrescaler  = 0x2B,
    TReloadH    = 0x2C,
    TReloadL    = 0x2D,

    Version     = 0x37,

    AnalogTest  = 0x38,

    TestBus     = 0x3A,

    TestSel1    = 0x31,
    TestSel2    = 0x32,

    TestPinEn   = 0x33,
    TestPinValue = 0x34,

    TestDAC1    = 0x36,

    TestDAC2    = 0x37
};