#include "DDSHeader.h"

namespace HAYDEN
{
    int DDSHeaderBuilder::ComputePitch(int width, int height, int decompressedSize, enum ImageType imageType)
    {
        int pitch = 0;
        if (imageType == FMT_RG8 || imageType == FMT_RGBA8) // RG8, RGBA8
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

    DDSHeaderBuilder::DDSHeaderBuilder(int width, int height, int decompressedSize, enum ImageType imageType)
    {
        _DDSHeader.ddsHeight = height;
        _DDSHeader.ddsWidth = width;
        _DDSHeader.ddsPitch = decompressedSize;

        // This checks imageTypes according to the frequency they are used in the game.
        // If an imageType is rarely used, it is at the bottom of this list.
        switch (imageType)
        {
            case FMT_BC1_ZERO_ALPHA:
            case FMT_BC1_SRGB:
            case FMT_BC1_LINEAR:
                // use defaults
                return;
            case FMT_BC4_LINEAR:
                _DDSHeader.ddsType = 1429488450;    // BC4U
                return;
            case FMT_BC5_LINEAR:
                _DDSHeader.ddsType = 1429553986;    // BC5U
                return;
            case FMT_BC7_SRGB:
            case FMT_BC7_LINEAR:
                _AppendDXT10 = 1;
                _DDSHeader.ddsType = 808540228;     // DX10
                return;
            case FMT_BC3_SRGB:
            case FMT_BC3_LINEAR:
                _DDSHeader.ddsType = 894720068;     // DXT5
                return;
            case FMT_RGBA8:
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
            case FMT_ALPHA:
                _DDSHeader.ddsType = 0;
                _DDSHeader.flags = 135183;
                _DDSHeader.pfFlags = 2;             // DDPF_ALPHA
                _DDSHeader.rgbBits = 8;
                _DDSHeader.aBitMsk = 255;
                _DDSHeader.ddsPitch = ComputePitch(width, height, decompressedSize, imageType);
                return;
            case FMT_RG8:
                _DDSHeader.ddsType = 0;
                _DDSHeader.flags = 135183;          // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT
                _DDSHeader.pfFlags = 131073;        // DDPF_ALPHAPIXELS | DDPF_LUMINANCE
                _DDSHeader.rgbBits = 16;
                _DDSHeader.rBitMsk = 255;
                _DDSHeader.aBitMsk = 65280;
                _DDSHeader.ddsPitch = ComputePitch(width, height, decompressedSize, imageType);
                return;
            case FMT_BC6H_UF16:
                _AppendDXT10 = 1;
                _DDSHeader.ddsType = 808540228;     // DX10
                _DDSHeaderDXT10.dxgiFormat = 95;    // DXGI_FORMAT_BC6H_UF16
                return;
            default:
                return;
        }
    }
}