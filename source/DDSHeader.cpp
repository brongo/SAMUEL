#include "DDSHeader.h"

namespace HAYDEN
{
    int DDSHeaderBuilder::ComputePitch(int width, int height, int decompressedSize, int imageType)
    {
        int pitch = 0;
        if (imageType == 7 || imageType == 3) // RG8, RGBA8
        {
            pitch = (width * 16 + 7) / 8;
        }
        return pitch;
    }
    std::vector<byte> DDSHeaderBuilder::ConvertToByteVector()
    {
        auto ptr = reinterpret_cast<byte*>(&_DDSHeader);
        auto buffer = std::vector<byte>(ptr, ptr + sizeof(_DDSHeader));

        if (_AppendDXT10)
        {
            auto ptrDXT10 = reinterpret_cast<byte*>(&_DDSHeaderDXT10);
            auto bufferDXT10 = std::vector<byte>(ptrDXT10, ptrDXT10 + sizeof(_DDSHeaderDXT10));
            buffer.insert(std::end(buffer), std::begin(bufferDXT10), std::end(bufferDXT10));
        }
        return buffer;
    }

    DDSHeaderBuilder::DDSHeaderBuilder(int width, int height, int decompressedSize, int imageType)
    {
        _DDSHeader.ddsHeight = height;
        _DDSHeader.ddsWidth = width;
        _DDSHeader.ddsPitch = decompressedSize;

        // Might want to make this a switch statement, not sure...
        // This checks imageTypes according to the frequency they are used in the game.
        // If an imageType is rarely used, it is at the bottom of this list. 

        // FMT_BC1_ZERO_ALPHA, FMT_BC1_SRGB, or FMT_BC1 (Linear)
        if (imageType == 54 || imageType == 33 || imageType == 10)
        {
            // use defaults
            return;
        }
            
        // FMT_BC4
        if (imageType == 24)
        {
            _DDSHeader.ddsType = 1429488450;    // BC4U
            return;
        }

        // FMT_BC5
        if (imageType == 25)
        {
            _DDSHeader.ddsType = 1429553986;    // BC5U
            return;
        }

        // FMT_BC7_SRGB or FMT_BC7 (Linear)
        if (imageType == 35 || imageType == 23)
        {
            _AppendDXT10 = 1;
            _DDSHeader.ddsType = 808540228;     // DX10
            return;
        }

        // FMT_BC3_SRGB or FMT_BC3 (Linear)
        if (imageType == 34 || imageType == 11)
        {
            _DDSHeader.ddsType = 894720068;     // DXT5
            return;
        }

        // FMT_RGBA8
        if (imageType == 3)
        {
            _DDSHeader.ddsType = 0;
            _DDSHeader.flags = 135183;          // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT
            _DDSHeader.pfFlags = 65;            // DDPF_ALPHAPIXELS | DDPF_RGB
            _DDSHeader.rgbBits = 32;
            _DDSHeader.rBitMsk = 16711680;
            _DDSHeader.gBitMsk = 65280;
            _DDSHeader.bBitMsk = 255;
            _DDSHeader.aBitMsk = -16777216;
            _DDSHeader.ddsPitch = ComputePitch(width, height, decompressedSize, imageType);
            return;
        }

        // FMT_ALPHA
        if (imageType == 5)
        {
            _DDSHeader.ddsType = 0;
            _DDSHeader.flags = 135183;
            _DDSHeader.pfFlags = 2;             // DDPF_ALPHA
            _DDSHeader.rgbBits = 8;
            _DDSHeader.aBitMsk = 255;
            _DDSHeader.ddsPitch = ComputePitch(width, height, decompressedSize, imageType);
            return;
        }

        // FMT_RG8
        if (imageType == 7)
        {
            _DDSHeader.ddsType = 0;
            _DDSHeader.flags = 135183;          // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT
            _DDSHeader.pfFlags = 131073;        // DDPF_ALPHAPIXELS | DDPF_LUMINANCE
            _DDSHeader.rgbBits = 16;
            _DDSHeader.rBitMsk = 255;
            _DDSHeader.aBitMsk = 65280;
            _DDSHeader.ddsPitch = ComputePitch(width, height, decompressedSize, imageType);
            return;
        }

        // FMT_BC6H_UF16
        if (imageType == 22)
        {
            _AppendDXT10 = 1;
            _DDSHeader.ddsType = 808540228;     // DX10
            _DDSHeaderDXT10.dxgiFormat = 95;    // DXGI_FORMAT_BC6H_UF16
            return;
        }

        return;
    }
}