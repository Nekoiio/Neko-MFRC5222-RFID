// ============================================================================
// MFRC522.cpp
//
// Implements the high-level driver for the NXP MFRC522 RFID reader.
//
// Layer responsibilities:
//
// Application
//     ↓
// MFRC522        <-- This file
//     ↓
// RegisterAccess <-- Encodes SPI register reads/writes
//     ↓
// SPIBus         <-- Hardware abstraction (Arduino, STM32, etc.)
//
// The MFRC522 class is responsible for:
//  - Device initialization
//  - RF field control
//  - Command execution
//  - FIFO management
//  - Interrupt polling
//  - Register manipulation helpers
//
// It is NOT responsible for:
//  - SPI implementation
//  - Timing implementation
//  - GPIO management
// ============================================================================
#include "MFRC522.hpp"

//! Constructor
MFRC522::MFRC522(RegisterAccess& registers)
    : registers(registers)
{}

//! begin
bool MFRC522::begin()
{
    // Prevent reinitializing the chip if begin() is accidentally called twice.
    if(isInitialized)
        return true;

    // Reset the chip into a known state and apply the recommended configuration.
    reset();

    // Verify SPI communication by reading the Version register.
    // A valid version means the reader is responding correctly.
    return isInitialized = checkVersion();
}

//! initRegisters
void MFRC522::initRegisters()
{
    // -------------------------------------------------------------------------
    // These values come from the MFRC522 datasheet's recommended initialization
    // sequence. They configure the internal timer, CRC behavior, modulation,
    // and prepare the chip for ISO14443-A (MIFARE Classic) communication.
    // -------------------------------------------------------------------------

    // Timer configuration.
    // Used internally by the MFRC522 for command timeouts and communication timing.
    registers.writeRegister(Register::TMode,       0x80);
    registers.writeRegister(Register::TPrescaler,  0xA9);
    registers.writeRegister(Register::TReloadH,    0x03);
    registers.writeRegister(Register::TReloadL,    0xE8);

    // RF transmission configuration.

    // Force 100% ASK modulation.
    // Required by the ISO14443-A specification used by MIFARE cards.
    registers.writeRegister(Register::TxASK, 0x40);

    // Configure the CRC preset and communication mode expected by ISO14443-A.
    registers.writeRegister(Register::Mode, 0x3D);

    // Finally enable the RF field so cards can actually be powered.
    setAntennaOn();
}

//! reset
void MFRC522::reset()
{
    // Tell the MFRC522 to perform an internal software reset.
    // This clears FIFOs, stops active commands and restores default registers.
    registers.writeRegister(
        Register::Command,
        static_cast<uint8_t>(Command::Reset)
    );

    // Wait until the chip finishes resetting.
    // During reset the PowerDown bit remains set.
    while(registers.readRegister(Register::Command) & 0b00010000)
    {
        // TODO:
        // Replace this busy wait with a timeout.
        // If the chip is disconnected or damaged this loop would never exit.
    }

    // Apply the recommended configuration after every reset.
    initRegisters();
}

//! getVersion
uint8_t MFRC522::getVersion()
{
    // Read the silicon revision.
    // Typical values:
    // 0x91 = Version 1.0
    // 0x92 = Version 2.0
    return registers.readRegister(Register::Version);
}

//! setAntennaOn
bool MFRC522::setAntennaOn()
{
    // Avoid unnecessary register writes.
    if(isAntennaOn)
        return true;

    uint8_t value =
        registers.readRegister(Register::TxControl);

    // Bits 0 and 1 enable the transmitter outputs (TX1/TX2).
    // Only modify them if they are currently disabled so we preserve
    // every other configuration bit inside TxControl.
    if((value & 0b00000011) != 0b00000011)
    {
        value |= 0b00000011;

        registers.writeRegister(
            Register::TxControl,
            value
        );
    }

    isAntennaOn = true;

    return true;
}

//! setAntennaOff
bool MFRC522::setAntennaOff()
{
    // Clearing bits 0 and 1 disables both RF transmitter outputs.
    // This removes the RF field without affecting the remaining register bits.
    clearBitMask(Register::TxControl, 0b00000011);

    isAntennaOn = false;

    return true;
}

//! transceive
bool MFRC522::transceive(
    const uint8_t* sendData,
    size_t sendLength,
    uint8_t* response,
    size_t& responseLength,
    uint8_t validBits

)
{
    // Ensure the previous command has completely stopped before
    // starting a new transaction.
    registers.writeRegister(
        Register::Command,
        static_cast<uint8_t>(Command::Idle)
    );

    // Start from a clean state.
    clearFIFO();      // Remove stale FIFO contents.
    clearComIrq();    // Clear interrupt flags from previous commands.

    // Load the outgoing packet into the MFRC522 FIFO.
    writeToFIFO(sendData, sendLength);

    // Tell the MFRC522 that the next operation is a Transceive.
    registers.writeRegister(
        Register::Command,
        static_cast<uint8_t>(Command::Transceive)
    );

    // Setting StartSend actually begins RF transmission.
    // Simply writing Command::Transceive is NOT enough.
    // Configure how many bits of the last byte should be transmitted.
    registers.writeRegister(
        Register::BitFraming,
        validBits & 0x07    // Only the 3 least significant bits are valid for txreg. This prevents accidentally overwriting other bits in the BitFraming register.
    );

    // Start transmission by setting the StartSend bit.
    setBitMask(Register::BitFraming, 0b10000000);

    if (!waitForCompletion())
    {
        clearBitMask(Register::BitFraming, 0b10000000);

        registers.writeRegister(
         Register::Command,
         static_cast<uint8_t>(Command::Idle)
        );

        registers.writeRegister(Register::BitFraming, 0x00);

        return false;
    }

    // Stop transmission.
    clearBitMask(Register::BitFraming, 0b10000000);

    // Restore normal byte-aligned transmission.
    registers.writeRegister(Register::BitFraming, 0x00);
    // Explicitly stop the command so the chip is ready for the next transaction.
    registers.writeRegister(
        Register::Command,
        static_cast<uint8_t>(Command::Idle)
    );

    // Verify the hardware didn't detect protocol, parity, CRC or FIFO errors.
    if(!checkForErrors())
    {
        return false;
    }

    // Read the received bytes from the FIFO.
    // The helper returns how many bytes were actually available.
    responseLength = readFromFIFO(response, responseLength);

    return true;
}

//! Privates

bool MFRC522::checkVersion()
{
    uint8_t version = getVersion();

    // 0x00 usually means no communication.
    // 0xFF usually means the SPI bus is floating.
    // Any other documented value indicates the chip is alive.
    return !(version == 0x00 || version == 0xFF);
}

//! Transceive Specific Functions -----------------------------------------------

void MFRC522::clearComIrq()
{
    // ComIrqReg is a "write 1 to clear" register.
    // Writing 1 clears the corresponding interrupt flag.
    registers.writeRegister(Register::ComIrq, 0b01111111);
}

void MFRC522::clearFIFO()
{
    // Bit 7 (FlushBuffer) clears the FIFO and resets its internal read/write
    // pointers. The FIFO level counter is also reset.
    registers.writeRegister(Register::FIFOLevel, 0b10000000);
}

void MFRC522::writeToFIFO(uint8_t* data, size_t length)
{
    // Every write appends one byte.
    // The MFRC522 automatically increments the FIFO pointer internally.



    for(size_t i = 0; i < length; i++)
    {
        registers.writeRegister(Register::FIFOData, data[i]);
        //registers.writeRegister(Register::FIFOData, 0x00);      // Padding for the idk, maybe its a bug somewhere //!Fixed
    }
}

size_t MFRC522::readFromFIFO(uint8_t* buffer, size_t max_length)
{
    // Determine how many bytes the MFRC522 actually received.
    size_t availableBytes = registers.readRegister(Register::FIFOLevel);

    // Never read more than the caller's buffer can hold.
    size_t read_up_to =
        (availableBytes < max_length)
            ? availableBytes
            : max_length;

    // Reading FIFOData automatically advances the FIFO read pointer.
    for(size_t i = 0; i < read_up_to; i++)
    {
        buffer[i] = registers.readRegister(Register::FIFOData);
    }

    return read_up_to;
}

bool MFRC522::waitForCompletion()
{
    // Poll the interrupt register until the command either completes
    // or fails. A future improvement is to add a timeout so this
    // loop cannot run forever.

    while(true)
    {
        uint8_t irqStatus =
            registers.readRegister(Register::ComIrq);

        // Internal timer expired before the command completed.
        if(irqStatus & 0b00000001)
        {
            return false;
        }

        // The MFRC522 detected a communication error.
        if(irqStatus & 0b00000010)
        {
            return false;
        }

        // RxIRq or IdleIRq indicates the command has completed.
        if(irqStatus & 0b00110000)
        {
            return true;
        }
    }
}

void MFRC522::setBitMask(Register reg, uint8_t mask)
{
    // Read-modify-write helper.
    // Used when only certain bits should change while preserving the rest.
    uint8_t currentValue = registers.readRegister(reg);
    registers.writeRegister(reg, currentValue | mask);
}

void MFRC522::clearBitMask(Register reg, uint8_t mask)
{
    // Read-modify-write helper for clearing specific bits.
    // Every bit outside the mask remains unchanged.
    uint8_t currentValue = registers.readRegister(reg);
    registers.writeRegister(reg, currentValue & ~mask);
}

bool MFRC522::checkForErrors()
{
    // ErrorReg contains hardware-reported communication errors such as
    // CRC failures, parity errors, collisions and FIFO overflows.
    // For now we simply treat any non-zero value as a failed transaction.
    //TODO implement constexpr bitmasks for each error type and return a more specific error code.
    uint8_t errors = registers.readRegister(Register::Error);

    return (errors == 0);
}