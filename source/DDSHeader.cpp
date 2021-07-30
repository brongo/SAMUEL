#include "DDSHeader.h"

namespace HAYDEN
{
    std::vector<byte> DDSHeaderBuilder::ConvertToByteVector()
    {
        auto ptr = reinterpret_cast<byte*>(&_DDSHeader);
        auto buffer = std::vector<byte>(ptr, ptr + sizeof(_DDSHeader));
        return buffer;
    }

    DDSHeaderBuilder::DDSHeaderBuilder(int width, int height, int decompressedSize, int imageType)
    {
        _DDSHeader.ddsHeight = height;
        _DDSHeader.ddsWidth = width;
        _DDSHeader.ddsPitch = decompressedSize;

        // BC5
        if (imageType == 25)
        { 
            _DDSHeader.ddsType = 843666497; // ATI2
            _DDSHeader.rgbBits = 1498952257;  // A2XY
        }
        return;
    }
}