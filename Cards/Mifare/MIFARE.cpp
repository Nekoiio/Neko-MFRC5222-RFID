#include "MIFARE.hpp"



MIFARE::MIFARE(MFRC522& reader)
    : reader(reader)
{}

bool MIFARE::authenticate(
    uint8_t blockAddress,
    Command keyType,
    const uint8_t* key,
    const uint8_t uid[4],
    size_t uidLength
)
{
    //TODO
}
    // The authentication command requires the following data:
    // Handle REQA/WUPA and anticollision before calling this function.
    //   MIFARE command (0x60 for Key A, 0x61 for Key B)
    //  - 1 byte: Key type (A or B)
    //  - 1 byte: Block address to authenticate
    //  - 6 bytes: Key (A or B)
    //  - 4 bytes: UID (only the first 4 bytes are used for authentication)




    
MIFAREStatus MIFARE::REQA(uint8_t (&atqa)[ATQA_SIZE])
{
    return sendRequest(
        MIFARECommand::REQA,
        atqa
    );
}


MIFAREStatus MIFARE::WUPA(uint8_t (&atqa)[ATQA_SIZE])
{
    return sendRequest(
        MIFARECommand::WUPA,
        atqa
    );
}


MIFAREStatus MIFARE::sendRequest(
    MIFARECommand command,
    uint8_t (&atqa)[ATQA_SIZE]
)
{
    const uint8_t request[] =
    {
        static_cast<uint8_t>(command)
    };

    size_t responseLength = ATQA_SIZE;

    bool success = reader.transceive(
        request,
        sizeof(request),
        atqa,
        responseLength,
        7
    );

    if(!success)
    {
        return MIFAREStatus::Timeout;
    }

    if(responseLength != ATQA_SIZE)
    {
        return MIFAREStatus::ProtocolError;
    }

    return MIFAREStatus::Ok;
}