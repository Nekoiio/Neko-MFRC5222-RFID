#pragma once

#include <stdint.h>
#include <stddef.h>

#include "RegisterAccess.hpp"
#include "Registers/Registers.hpp"
#include "Registers/Commands.hpp"

class MFRC522
{
    public:
        explicit MFRC522(RegisterAccess& registers);

        bool begin();

        void reset();

        uint8_t getVersion();

        bool setAntennaOn();

        bool setAntennaOff();

        bool transceive(
            const uint8_t* sendData,
            size_t sendLength,
            uint8_t* response,
            size_t& responseLength,
            uint8_t validbits = 0
        );
    
    private:

        void initRegisters();

        bool checkVersion();

        RegisterAccess& registers;

        bool isAntennaOn = false;
        bool isInitialized = false;

        void clearFIFO();
        void clearComIrq();

        void writeToFIFO(uint8_t* data, size_t length);
        size_t readFromFIFO(uint8_t* buffer, size_t length);

        bool waitForCompletion();
        bool checkForErrors();

        void setBitMask(Register reg, uint8_t mask);
        void clearBitMask(Register reg, uint8_t mask);




};