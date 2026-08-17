#include "ArduinoSPIBus.hpp"
#include <Arduino.h>
#include <SPI.h>

ArduinoSPIBus::ArduinoSPIBus(uint8_t chipSelectPin) : csPin(chipSelectPin)  {}

void ArduinoSPIBus::begin()
{
    SPI.begin();

    pinMode(csPin, OUTPUT);

    digitalWrite(csPin, HIGH);
}


void ArduinoSPIBus::beginTransaction()
{
    SPI.beginTransaction( // i just copy pasted this from google lowkey dont know. Just know that MFRC522 expects this
        SPISettings(
            4000000,    // Clock speed
            MSBFIRST,   // Bit order
            SPI_MODE0   // Clock polarity + phase
        )
    );

    digitalWrite(csPin, LOW); // LOW = listen to me | Asserts chip select
}

void ArduinoSPIBus::endTransaction()
{
    digitalWrite(csPin, HIGH); // HIGH = complete | Release chip select
 
    SPI.endTransaction();
}


uint8_t ArduinoSPIBus::transfer(uint8_t byte)
{                                              // Single byte transfrs
    return SPI.transfer(byte);
}

void ArduinoSPIBus::transfer(
    uint8_t* buffer,
    size_t length
)
{
    beginTransaction();
    for(size_t i = 0; i < length; i++)
    {
        buffer[i] = SPI.transfer(buffer[i]);
    }
    endTransaction();
}