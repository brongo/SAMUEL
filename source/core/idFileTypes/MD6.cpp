#include "MD6.h"

namespace HAYDEN
{
    void MD6::Serialize(const std::vector<uint8_t> binaryData)
    {
        Footer = *(MD6_FOOTER*)(binaryData.data() + (binaryData.size() - sizeof(MD6_FOOTER)));

        for (uint64_t i = 0; i < 3; i++)
        {
            uint64_t offset = sizeof(MD6_FOOTER) + (sizeof(MD6_STREAMDB_DATA) * (3 - i));
            StreamDBData[i] = *(MD6_STREAMDB_DATA*)(binaryData.data() + (binaryData.size() - offset));
        }
        
        // Temporary for rev-eng
        RawMD6Header.insert(RawMD6Header.begin(), binaryData.begin(), binaryData.end());
        return;
    }
}