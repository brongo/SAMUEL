#pragma once

#include <vector>
#include <algorithm>

namespace HAYDEN
{
	typedef unsigned char byte;
	struct DDSHeader
	{
		const int magic = 542327876;	// "DDS "
		const int hsize = 124;

		int flags = 659463;				// DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE
		int ddsHeight = 0;
		int ddsWidth = 0;
		int ddsPitch = 0;

		const int depth = 1;
		const int numMips = 1;
		const int null[11] = { 0 };
		const int pfSize = 32;

		int pfFlags = 4;				// DDPF_FOURCC
		int ddsType = 827611204;		// DXT1
		int rgbBits = 0;
		int rBitMsk = 0;
		int gBitMsk = 0;
		int bBitMsk = 0;
		int aBitMsk = 0;

		const int caps1 = 4096;			// DDSCAPS_TEXTURE
		const int caps2 = 0;
		const int caps3 = 0;
		const int caps4 = 0;
		const int caps5 = 0;
	};

	struct DDSHeaderDXT10
	{
		int dxgiFormat = 98;			// DXGI_FORMAT_BC7_UNORM
		const int rsDimension = 3;		// D3D10_RESOURCE_DIMENSION_TEXTURE2D
		const int miscFlag1 = 0;
		const int arraySize = 1;
		const int miscFlag2 = 0;		// DDS_ALPHA_MODE_UNKNOWN
	};

	class DDSHeaderBuilder
	{	
		public:
			int ComputePitch(int width, int height, int decompressedSize, int imageType);
			std::vector<byte> ConvertToByteVector();
			DDSHeaderBuilder(int width, int height, int decompressedSize, int imageType);
			
		private:
			DDSHeader _DDSHeader;
			DDSHeaderDXT10 _DDSHeaderDXT10;
			bool _AppendDXT10 = 0;
	};
}