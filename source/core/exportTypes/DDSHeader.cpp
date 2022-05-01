#include "DDSHeader.h"

namespace HAYDEN
{
    int DDSHeaderBuilder::ComputePitch(int width, int height, int decompressedSize, ImageType imageType)
    {
        int pitch = 0;
        if (imageType == ImageType::FMT_RG8 || imageType == ImageType::FMT_RGBA8) // RG8, RGBA8
        {
            pitch = (width * 16 + 7) / 8;
        }
        return pitch;
    }
    std::vector<uint8_t> DDSHeaderBuilder::ConvertToByteVector()
    {
        auto ptr = reinterpret_cast<uint8_t*>(&_DDSHeader);
        auto buffer = std::vector<uint8_t>(ptr, ptr + sizeof(_DDSHeader));

        if (_AppendDXT10)
        {
            auto ptrDXT10 = reinterpret_cast<uint8_t*>(&_DDSHeaderDXT10);
            auto bufferDXT10 = std::vector<uint8_t>(ptrDXT10, ptrDXT10 + sizeof(_DDSHeaderDXT10));
            buffer.insert(std::end(buffer), std::begin(bufferDXT10), std::end(bufferDXT10));
        }
        return buffer;
    }
    DDSHeaderBuilder::DDSHeaderBuilder(int width, int height, int decompressedSize, ImageType imageType)
    {
        _DDSHeader.ImgHeight = height;
        _DDSHeader.ImgWidth = width;
        _DDSHeader.ImgPitch = decompressedSize;

        // imageTypes are listed according to the frequency they are used in the game.
        // If an imageType is rarely used, it is at the bottom of this list.
        switch (imageType)
        {
            case ImageType::FMT_BC1_ZERO_ALPHA:
            case ImageType::FMT_BC1_SRGB:
            case ImageType::FMT_BC1_LINEAR:
                // use defaults
                return;
            case ImageType::FMT_BC4_LINEAR:
                _DDSHeader.DDSType = 1429488450;    // BC4U
                return;
            case ImageType::FMT_BC5_LINEAR:
                _DDSHeader.DDSType = 843666497;     // ATI2
                return;
            case ImageType::FMT_BC7_SRGB:
            case ImageType::FMT_BC7_LINEAR:
                _AppendDXT10 = 1;
                _DDSHeader.DDSType = 808540228;     // DX10
                return;
            case ImageType::FMT_BC3_SRGB:
            case ImageType::FMT_BC3_LINEAR:
                _DDSHeader.DDSType = 894720068;     // DXT5
                return;
            case ImageType::FMT_RGBA8:
                _DDSHeader.DDSType = 0;
                _DDSHeader.ImgFlags = 135183;       // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT
                _DDSHeader.PfFlags = 65;            // DDPF_ALPHAPIXELS | DDPF_RGB
                _DDSHeader.RGBBits = 32;
                _DDSHeader.RBitMsk = 16711680;
                _DDSHeader.GBitMsk = 65280;
                _DDSHeader.BBitMsk = 255;
                _DDSHeader.ABitMsk = -16777216;
                _DDSHeader.ImgPitch = ComputePitch(width, height, decompressedSize, imageType);
                return;
            case ImageType::FMT_ALPHA:
                _DDSHeader.DDSType = 0;
                _DDSHeader.ImgFlags = 135183;
                _DDSHeader.PfFlags = 2;             // DDPF_ALPHA
                _DDSHeader.RGBBits = 8;
                _DDSHeader.ABitMsk = 255;
                _DDSHeader.ImgPitch = ComputePitch(width, height, decompressedSize, imageType);
                return;
            case ImageType::FMT_RG8:
                _DDSHeader.DDSType = 0;
                _DDSHeader.ImgFlags = 135183;       // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT
                _DDSHeader.PfFlags = 131073;        // DDPF_ALPHAPIXELS | DDPF_LUMINANCE
                _DDSHeader.RGBBits = 16;
                _DDSHeader.RBitMsk = 255;
                _DDSHeader.ABitMsk = 65280;
                _DDSHeader.ImgPitch = ComputePitch(width, height, decompressedSize, imageType);
                return;
            case ImageType::FMT_BC6H_UF16:
                _AppendDXT10 = 1;
                _DDSHeader.DDSType = 808540228;     // DX10
                _DDSHeaderDXT10.DXGIFormat = 95;    // DXGI_FORMAT_BC6H_UF16
                return;
            default:
                return;
        }
    }
}