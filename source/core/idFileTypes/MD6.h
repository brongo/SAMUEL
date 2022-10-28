#pragma once

#include <string>
#include <vector>
#include <cmath>

#include "../Common.h"
#include "StreamDBGeometry.h"

#pragma pack(push)	// Not portable, sorry.
#pragma pack(1)		// Works on my machine (TM).

namespace HAYDEN
{
    struct MD6_UNK_FLOATS
    {
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

    struct MD6_BONE_INFO
    {
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

    struct MD6_MESH_UNKNOWNS
    {
        uint8_t UnkChar1 = 0;
        uint32_t UnkDummy1 = 0xFFFFFFFF;
        uint32_t UnkInt1 = 0;
        uint32_t UnkInt2 = 0;
        uint32_t UnkInt3 = 0;
        uint32_t UnkInt4 = 0;
    };

    struct MD6_LOD_INFO
    {
        uint32_t NullPad32 = 0;
        uint32_t NumVertices = 0;
        uint32_t NumFaces = 0;

        GEO_METADATA Meta;
        GEO_FLAGS Flags;

        float_t UnkFloat1 = 0;
        float_t UnkFloat2 = 0;
    };

    struct MD6_MESH_FOOTER
    {
        uint32_t UnkInt1 = 0;
        uint32_t UnkInt2 = 0;
        uint32_t UnkInt3 = 0;
        uint8_t UnkChar1 = 0;
    };

    struct MD6_MESH_INFO
    {
        uint32_t MeshNameStrlen = 0;
        std::string MeshName;
        uint32_t MaterialDeclStrlen = 0;
        std::string MaterialDeclName;
        MD6_MESH_UNKNOWNS MeshUnknowns;
        std::vector<MD6_LOD_INFO> LODInfo;
        MD6_MESH_FOOTER MeshFooter;
        uint32_t UnkTuple[2] = { 0 };               // Only if MD6_BONE_INFO.UnkFloat7 and UnkFloat8 == 0
    };

    struct MD6_STREAMDB_HEADER
    {
        uint16_t MD6Version = 60;                   // 60 is normal, 124 is uvlayout_lightmap = 1
        uint16_t MD6Version2 = 2;                   // always 2?
        uint32_t DecompressedSize = 0;
        uint32_t NumOffsets = 4;                    // always 4 (or 5 if uvlayout_lightmap = 1)
    };

    struct MD6_STREAMDB_DATA
    {
        uint32_t UnkNormalInt = 32;
        uint32_t UnkUVInt = 20;
        uint32_t UnkColorInt = 131072;
        uint32_t UnkFacesInt = 8;
        uint32_t UnkInt99 = 0;
        uint32_t NormalStartOffset = 0;
        uint32_t UVStartOffset = 0;
        uint32_t ColorStartOffset = 0;
        uint32_t FacesStartOffset = 0;
    };

    struct MD6_STREAMDB_DATA_VARIANT
    {
        uint32_t UnkNormalInt = 32;
        uint32_t UnkUVInt = 20;
        uint32_t UnkUVLightmapInt = 64;             // 124 variant only
        uint32_t UnkColorInt = 131072;
        uint32_t UnkFacesInt = 8;
        uint32_t UnkInt99 = 0;
        uint32_t NormalStartOffset = 0;
        uint32_t UVStartOffset = 0;
        uint32_t UVLightMapOffset = 0;          // 124 variant only
        uint32_t ColorStartOffset = 0;
        uint32_t FacesStartOffset = 0;
    };

    struct MD6_GEOMETRY_STREAMDISK_LAYOUT
    {
        uint32_t StreamCompressionType = 3;         // 3 = NONE_MODEL, 4 = KRAKEN_MODEL
        uint32_t DecompressedSize = 0;
        uint32_t CompressedSize = 0;
        uint32_t CumulativeStreamDBCompSize = 0;
    };

    // Extracted from *.resources files
    class MD6_HEADER
    {
	public:

            uint32_t MD6SKL_Filename_Length = 0;
            std::string MD6SKL_Filename;
            MD6_UNK_FLOATS MD6UnkFloats;
            
            uint16_t NumBones = 0;
            std::vector<uint8_t> BoneNumbers;
            MD6_BONE_INFO BoneInfo;

            uint32_t NumMeshes = 0;
            std::vector<MD6_MESH_INFO> MeshInfo;

            int NumStreams = 5;
            std::vector<MD6_STREAMDB_HEADER> StreamDBHeaders;
            std::vector<MD6_STREAMDB_DATA> StreamDBData;
            std::vector<MD6_STREAMDB_DATA_VARIANT> StreamDBDataVariant;
            std::vector<MD6_GEOMETRY_STREAMDISK_LAYOUT> StreamDiskLayout;

	    std::vector<uint8_t> RawMD6Header; // Temporary for rev-eng		
	    void ReadBinaryHeader(const std::vector<uint8_t> binaryData);
    };

    class MD6
    {
        public:
            MD6_HEADER Header;
            std::vector<Mesh> MeshGeometry;
            void Serialize(MD6_HEADER md6Header, std::vector<uint8_t> md6Geo);
    };
}

#pragma pack(pop)