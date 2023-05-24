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
    struct LWO_METADATA
    {
	uint64_t NullPad64_0 = 0;
	uint32_t NullPad32_0 = 0;
	uint32_t FileType = 0;
	uint32_t NumMeshes = 0;
	uint32_t NullPad32_1 = 0;
	uint64_t UnkHash = 0;		        // if zero, uses UnkTuple[2] at end of BML headers
	uint32_t NullPad32_2 = 0;
    };

    struct LWO_MESH_HEADER
    {
        uint32_t UnkInt1 = 0;
        uint32_t UnkInt2 = 0;
        uint32_t DeclStrlen = 0;
    };

    struct LWO_MESH_HEADER_SHORT
    {
        uint32_t UnkInt2 = 0;
        uint32_t DeclStrlen = 0;
    };

    struct LWO_MESH_FOOTER
    {
        uint32_t UnkInt3 = 0;
        uint32_t Dummy1 = 0;
        uint32_t NullPad = 0;
    };

    struct LWO_LOD_INFO
    {
	uint32_t NullPad32 = 0;
	uint32_t DummyMask = 0;
        uint32_t NumVertices = 0;
        uint32_t NumEdges = 0;

        GeoFlags GeoFlags;
        GeoMetadata GeoMeta;

	float_t UnkFloat1 = 0;
	float_t UnkFloat2 = 0;
	float_t UnkFloat3 = 0;
	char Signature[4] = { 0 };
    };

    struct LWO_MESH_INFO
    {
        LWO_MESH_HEADER MeshHeader;
        std::string MaterialDeclName;
        LWO_MESH_FOOTER MeshFooter;
        std::vector<LWO_LOD_INFO> LODInfo;
        uint32_t UnkTuple[2] = { 0 };              // Only if LWO_HEADER.UnkHash == 0
    };
    
    struct LWO_STREAMDB_HEADER
    {
        uint32_t NumStreams = 1;
        uint16_t LWOVersion = 60;                   // 60 is normal, 124 is uvlayout_lightmap = 1
        uint16_t LWOVersion2 = 2;
        uint32_t DecompressedSize = 0;
        uint32_t NumOffsets = 4;                    // always 4 (or 5 if uvlayout_lightmap = 1)
    };

    struct LWO_STREAMDB_DATA
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

    struct LWO_STREAMDB_DATA_VARIANT
    {
        uint32_t UnkNormalInt = 32;
        uint32_t UnkUVLightmapInt = 64;             // 124 variant only
        uint32_t UnkUVInt = 20;
        uint32_t UnkColorInt = 131072;
        uint32_t UnkFacesInt = 8;
        uint32_t UnkInt99 = 0;
        uint32_t NormalStartOffset = 0;
        uint32_t UVLightMapOffset = 0;              // 124 variant only
        uint32_t UVStartOffset = 0;
        uint32_t ColorStartOffset = 0;
        uint32_t FacesStartOffset = 0;
    };

    struct LWO_GEOMETRY_STREAMDISK_LAYOUT
    {
        uint32_t StreamCompressionType = 3;         // 3 = NONE_MODEL, 4 = KRAKEN_MODEL
        uint32_t DecompressedSize = 0;
        uint32_t CompressedSize = 0;
        uint32_t CumulativeStreamDBCompSize = 0;
    };

    // Extracted from *.resources files
    class LWO_HEADER
    {
	public:
	    std::vector<uint8_t> RawLWOHeader;
	    void ReadBinaryHeader(const std::vector<uint8_t> binaryData);

            // Serialized file data
            LWO_METADATA Metadata;
            std::vector<LWO_MESH_INFO> MeshInfo;
            std::vector<LWO_STREAMDB_HEADER> StreamDBHeaders;
            std::vector<LWO_STREAMDB_DATA> StreamDBData;                         // Used if LWO version = 60
            std::vector<LWO_STREAMDB_DATA_VARIANT> StreamDBDataVariant;          // Used in LWO version = 124 (uv lightmap)
            std::vector<LWO_GEOMETRY_STREAMDISK_LAYOUT> StreamDiskLayout;

        private:

            // Defaults
            int  _BMLCount = 3;
            bool _UseExtendedBML = 0;
            bool _UseShortMeshHeader = 0;
            uint32_t _Num32ByteChunks = 0;
    };

    class LWO
    {
	public:
            LWO_HEADER Header;
            std::vector<Mesh> MeshGeometry;
	    void Serialize(LWO_HEADER lwoHeader, std::vector<uint8_t> lwoGeo);
    };
}

#pragma pack(pop)