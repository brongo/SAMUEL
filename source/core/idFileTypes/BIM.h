#pragma once

#include <vector>
#include <cmath>
#include <cstdint>

#pragma pack(push)	// Not portable, sorry.
#pragma pack(1)		// Works on my machine (TM).

namespace HAYDEN
{
    struct BIM_HEADER
    {
	uint8_t Signature[3] = { 0 };		// "BIM"
	uint8_t Version = 0;			// 0x15
	int32_t TextureType = 0;		// enum textureType_t
	int32_t TextureMaterialKind = 0;	// enum textureMaterialKind_t
	int32_t PixelWidth = 0;			// image width in pixels
	int32_t PixelHeight = 0;		// image height in pixels
	int32_t Depth = 0;
	int32_t MipCount = 0;			// determines # of MIPMAP structs to follow
	int64_t MipLevel = 0;
	float_t UnkFloat1 = 0;			// usually 1.0, purpose unknown
	uint8_t BoolIsEnvironmentMap = 0;	// 1 if image is an environment map
	int32_t TextureFormat = 0;		// enum textureFormat_t
	int32_t Always7 = 0;			// literally always 7, purpose unknown
	int32_t NullPadding = 0;
	int16_t AtlasPadding = 0;
	uint8_t BoolIsStreamed = 0;		// 1 if file is located in streamDB
	uint8_t UnkBoolA = 0;
	uint8_t BoolNoMips = 0;			// 1 if image has no mips
	uint8_t BoolFFTBloom = 0;		// 1 if using FFT Bloom
	int32_t StreamDBMipCount = 0;		// usually same number of mips stored in streamdb
    };

    struct BIM_MIPMAP
    {
	int64_t MipLevel = 0;		        // Starts at 0, increment by 1 each time it repeats
	int32_t MipPixelWidth = 0;		// Original PixelWidth reduced by 50% for each MipLevel
	int32_t MipPixelHeight = 0;		// Original PixelHeight reduced by 50% for each MipLevel
	int32_t UnkBoolB = 0;
	int32_t DecompressedSize = 0;	        // Decompressed size in bytes
	int32_t BoolIsCompressed = 0;	        // 1 if the texture is compressed
	int32_t CompressedSize = 0;		// Compressed size in bytes
	int32_t CumulativeSizeStreamDB = 0;
    };

    class BIM
    {
	public:
	    BIM_HEADER Header;
	    std::vector<BIM_MIPMAP> MipMaps;
	    std::vector<uint8_t> RawImageData;
	    void Serialize(const std::vector<uint8_t> binaryData);
    };
}

#pragma pack(pop)