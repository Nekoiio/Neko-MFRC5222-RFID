// ============================================================================
// RegisterAccess.cpp
//
// Implements the register-level communication layer for the MFRC522.
//
// Layer responsibilities:
//
// Application
//     ↓
// MFRC522
//     ↓
// RegisterAccess   <-- This file
//     ↓
// SPIBus
//     ↓
// Physical SPI Hardware
//
// This class is responsible for:
//   - Converting Register enums into SPI register addresses.
//   - Applying the MFRC522 SPI address transformations.
//   - Constructing register read/write packets.
//   - Hiding the SPI protocol from higher layers.
//
// This class is NOT responsible for:
//   - RFID commands (REQA, SELECT, AUTH, READ, WRITE)
//   - FIFO management
//   - Antenna control
//   - Timing
//   - Interrupt handling
//
// Why this layer exists:
//
//
// Before every register access the register address must be transformed:
//
//      Write:
//          ((Register << 1) & 0x7E)
//
//      Read:
//          ((Register << 1) & 0x7E) | 0x80
//
// ============================================================================

#include "RegisterAccess.hpp"
//! Constructor
RegisterAccess::RegisterAccess(SPIBus& spi)
    : spi(spi)
{}

//! WriteRegister
void RegisterAccess::writeRegister(Register reg, uint8_t value)
{
    // A register write always consists of two bytes:
    //
    // Byte 0 : Encoded register address
    // Byte 1 : Data to write
    //
    // Example:
    //
    // MOSI:
    // 0x02 0x3D
    //
    // Meaning:
    // Write 0x3D into ModeReg.

    uint8_t packet[2];

    packet[0] = encodeWrite(reg);
    packet[1] = value;

    spi.transfer(packet, 2);
}

//! ReadRegisters(more than 1 byte)
void RegisterAccess::readRegisters(
    Register reg,
    uint8_t* buffer,
    size_t length
)
{
    // Reading registers over SPI requires sending one address byte
    // followed by one dummy byte for every byte we want back.
    //
    // Example (read 3 bytes):
    //
    // MOSI:
    // [Address][00][00][00]
    //
    // MISO:
    // [xxxx][D0][D1][D2]
    //
    // The first received byte is meaningless because it is shifted out
    // while transmitting the register address.

   uint8_t packet[65];

    // First byte tells the MFRC522 which register to read.
    packet[0] = encodeRead(reg);

    // Remaining bytes are dummy bytes.
    // Their only purpose is to generate SPI clock pulses so the MFRC522
    // can shift its register data back to us.
    for(size_t i = 1; i < length + 1; i++)
    {
        packet[i] = 0x01;
    }

    // SPI is full duplex:
    //
    // - The packet array initially contains outgoing bytes.
    // - During transfer the received bytes overwrite the same buffer.
    spi.transfer(packet, length + 1);

    // Skip the first byte because it corresponds to the address phase.
    // The actual register data starts at packet[1].
    for(size_t i = 0; i < length; i++)
    {
        buffer[i] = packet[i + 1];
    }
}

//! ReadRegister(single byte)
uint8_t RegisterAccess::readRegister(Register reg)
{
    // Convenience wrapper around readRegisters().
    // Reuses the multi-byte implementation so all SPI read logic
    // exists in exactly one place.

    uint8_t value;

    readRegisters(reg, &value, 1);

    return value;
}

//! EncodeWrite
uint8_t RegisterAccess::encodeWrite(Register reg)
{
    // Convert the logical register address into the SPI write format.
    //
    // Transformation:
    //
    // Register Address
    //        ↓
    // Shift left by one
    //        ↓
    // Clear MSB (write operation)
    //
    // Datasheet:
    // SPI Address = (Register << 1) & 0x7E

    return (static_cast<uint8_t>(reg) << 1) & 0x7E;
}
//! EncodeRead
uint8_t RegisterAccess::encodeRead(Register reg)
{
    // Read uses the same transformation as write,
    // but sets bit 7 to indicate a read transaction.
    //
    // Datasheet:
    // SPI Address = ((Register << 1) & 0x7E) | 0x80

    return ((static_cast<uint8_t>(reg) << 1) & 0x7E) | 0x80;
}