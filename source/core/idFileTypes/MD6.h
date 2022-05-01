#pragma once

#include <string>
#include <vector>
#include <cmath>

#pragma pack(push)	// Not portable, sorry.
#pragma pack(1)		// Works on my machine (TM).

namespace HAYDEN
{
    struct MD6_HEADER
    {
	uint32_t MD6SKL_Filename_Length = 0;
	std::string MD6SKL_Filename;

	// Negative bounds? 
	float_t NegativeFloat1 = -1;
	float_t NegativeFloat2 = -1;
	float_t NegativeFloat3 = -1;

	// Positive bounds?
	float_t PositiveFloat1 = 1;
	float_t PositiveFloat2 = 1;
	float_t PositiveFloat3 = 1;

	uint8_t Always1 = 1;
	uint32_t NullPad = 0;
    };

    struct MD6_BONES
    {
	// BoneNumbers.size() = NumBones
	uint16_t NumBones = 0;
	std::vector<uint8_t> BoneNumbers;			

	// Negative bounds? 
	// Doesn't match the "bounds" in MD6_MESH_HEADER above.
	float_t NegativeBoneFloat1 = -1;
	float_t NegativeBoneFloat2 = -1;
	float_t NegativeBoneFloat3 = -1;
		
	// Positive bounds?
	// Doesn't match the "bounds" in MD6_MESH_HEADER above.
	float_t PositiveBoneFloat1 = 1;
	float_t PositiveBoneFloat2 = 1;
	float_t PositiveBoneFloat3 = 1;

	uint64_t Always5 = 5;

	// if these are zero, then texture/mesh section is shorter
	float_t UnkFloat7 = 0;
	float_t UnkFloat8 = 0;

	// Padding?
	uint8_t unkAlwaysNull[48] = { 0 };
    };

    struct MD6_MESH_HEADER
    {
	uint32_t NumMeshes = 0;
	uint32_t MeshNameHdr_Length = 0;
	std::string MeshNameHdr;
	uint32_t TextureName_Length = 0;
	std::string TextureName;
	std::vector<uint8_t> UnkMeshData;	// size = 118 bytes if MD6_BONES.UnkFloat7 == 0, otherwise size = 262 bytes
    };

    struct MD6_MESH_UNK
    {
	uint32_t MeshName_Length = 0;
	std::string MeshName;
	uint32_t Unk1 = 0;
	uint32_t Unk2 = 0;
	uint32_t Unk3 = 0;
    };

    struct MD6_MESH_LIST
    {
	uint32_t NumMeshesList = 0;
	std::vector<MD6_MESH_UNK> UnkMeshData;
    };

    struct MD6_UNK_CHUNK_48
    {
        uint8_t UnkChunk[48] = { 0 };
    };

    struct MD6_UNK_48_BYTE_CHUNKS
    {
	uint32_t NumUnknowns = 0;
	std::vector<MD6_UNK_CHUNK_48> UnknownChunks;
    };

    struct MD6_STREAMDB_DATA
    {
	uint32_t UnkStreamDB = 0;
	uint32_t DecompressedSize = 0;
	uint32_t CompressedSize = 0;
	uint32_t CumulativeCompressedSize = 0;
    };

    struct MD6_FOOTER
    {
	uint32_t NumLODs = 0;
	uint64_t NullPad0 = 0;
	uint32_t CumulativeFileSize = 0;
	uint32_t NumLODsStreamed = 0;
	uint64_t NullPad1 = 0;
	uint32_t CumulativeStreamDBSize = 0;
    };

    class MD6
    {
	public:

	    // Not implemented yet
	    // MD6_HEADER Header;
	    // MD6_BONES Bones;
	    // MD6_MESH_HEADER MeshHeader;
	    // MD6_MESH_LIST MeshList;
	    // MD6_UNK_48_BYTE_CHUNKS UnkChunks;

	    MD6_STREAMDB_DATA StreamDBData[3];
	    MD6_FOOTER Footer;	
	    std::vector<uint8_t> RawMD6Header; // Temporary for rev-eng		
	    void Serialize(const std::vector<uint8_t> binaryData);
    };
}

#pragma pack(pop)