#pragma once

#include "Core/MFRC522.hpp"
#include "MIFAREstatus.hpp"
#include "MIFAREcommands.hpp"

class MIFARE
{
    public:
        MIFARE(MFRC522& reader);

        static constexpr uint8_t KEY_SIZE = 6;
        static constexpr uint8_t BLOCK_SIZE = 16;
        static constexpr uint8_t ATQA_SIZE = 2;

        bool authenticate(
            uint8_t blockAddress,
            Command keyType,
            const uint8_t* key,
            const uint8_t uid[4],
            size_t uidLength
        );
        
        MIFAREStatus select();

        MIFAREStatus deselect();

        MIFAREStatus REQA(uint8_t (&atqa)[ATQA_SIZE]);

        MIFAREStatus WUPA(uint8_t (&atqa)[ATQA_SIZE]);



    private:

        MIFAREStatus sendRequest(
            MIFARECommand command,
            uint8_t (&atqa)[ATQA_SIZE]
        );

        MFRC522& reader;
};