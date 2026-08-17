#include "HAL/Arduino/ArduinoSPIBus.hpp"
#include "Registers/Registers.hpp"
#include "Registers/Commands.hpp"

#include "Core/RegisterAccess.hpp"
#include "Core/MFRC522.hpp"

// Chip Select pin
constexpr uint8_t SS_PIN = 10;

// Hardware Abstraction Layer
ArduinoSPIBus spi(SS_PIN);

// Register layer
RegisterAccess registers(spi);

// High-level driver
MFRC522 reader(registers);

void setup()
{
    Serial.begin(115200);

    while(!Serial)
    {
    }



    Serial.println();
    Serial.println("Initializing SPI...");

    spi.begin();

    Serial.println("Reading MFRC522 Version Register...");

    uint8_t version = reader.getVersion();

    Serial.print("Version = 0x");
    Serial.println(version, HEX);

    switch(version)
    {
        case 0x91:
            Serial.println("MFRC522 Version 1.0 detected.");
            break;

        case 0x92:
            Serial.println("MFRC522 Version 2.0 detected.");
            break;

        case 0x88:
            Serial.println("FM17522 clone detected.");
            break;

        case 0x00:
            Serial.println("No communication.");
            break;

        case 0xFF:
            Serial.println("SPI bus floating.");
            break;

        default:
            Serial.println("Unknown version.");
            break;
    }

    Serial.println("Testing Write...");
    uint8_t original = registers.readRegister(Register::Mode);

    Serial.print("Original: 0x");
    Serial.println(original, HEX);

    registers.writeRegister(Register::Mode, 0x3D);

    uint8_t updated = registers.readRegister(Register::Mode);

    Serial.print("Updated: 0x");
    Serial.println(updated, HEX);
}

void loop()
{
}