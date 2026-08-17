#pragma once

#include "SPIBus.hpp"

class ArduinoSPIBus : public SPIBus
{
public:

    explicit ArduinoSPIBus(uint8_t chipSelectPin);

    void begin() override;

    void endTransaction() override;

    void beginTransaction() override;

    uint8_t transfer(uint8_t byte) override;

    void transfer(
        uint8_t* buffer,
        size_t length
    ) override;

private:

    uint8_t csPin;
};