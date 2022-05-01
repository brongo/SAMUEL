#pragma once

#include <vector>
#include <algorithm>
#include <cstdint>

namespace HAYDEN
{
    struct DDSHeader
    {
	const int Magic = 542327876;	// "DDS "
	const int HeaderSize = 124;

	int ImgFlags = 659463;		// DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE
	int ImgHeight = 0;
	int ImgWidth = 0;
	int ImgPitch = 0;

	const int ImgDepth = 1;
	const int NumMips = 1;
	const int Null[11] = { 0 };
	const int PfSize = 32;

	int PfFlags = 4;		// DDPF_FOURCC
	int DDSType = 827611204;	// DXT1
	int RGBBits = 0;
	int RBitMsk = 0;
	int GBitMsk = 0;
	int BBitMsk = 0;
	int ABitMsk = 0;

	const int Caps1 = 4096;		// DDSCAPS_TEXTURE
	const int Caps2 = 0;
	const int Caps3 = 0;
	const int Caps4 = 0;
	const int Caps5 = 0;
    };

    struct DDSHeaderDXT10
    {
	int DXGIFormat = 98;		// DXGI_FORMAT_BC7_UNORM
	const int RsDimension = 3;	// D3D10_RESOURCE_DIMENSION_TEXTURE2D
	const int MiscFlag1 = 0;
	const int ArraySize = 1;
	const int MiscFlag2 = 0;	// DDS_ALPHA_MODE_UNKNOWN
    };

    enum class ImageType
    {
        FMT_BC1_ZERO_ALPHA = 54,
        FMT_BC7_SRGB = 35,
        FMT_BC3_SRGB = 34,
        FMT_BC1_SRGB = 33,
        FMT_BC5_LINEAR = 25, 
        FMT_BC4_LINEAR = 24, 
        FMT_BC7_LINEAR = 23, 
        FMT_BC6H_UF16 = 22, 
        FMT_BC3_LINEAR = 11, 
        FMT_BC1_LINEAR = 10,
        FMT_RG8 = 7, 
        FMT_ALPHA = 5, 
        FMT_RGBA8 = 3
    };

    class DDSHeaderBuilder
    {	
	public:
	    int ComputePitch(int width, int height, int decompressedSize, ImageType imageType);
	    std::vector<uint8_t> ConvertToByteVector();
	    DDSHeaderBuilder(int width, int height, int decompressedSize, ImageType imageType);
			
	private:
	    DDSHeader _DDSHeader;
	    DDSHeaderDXT10 _DDSHeaderDXT10;
	    bool _AppendDXT10 = 0;
    };
}