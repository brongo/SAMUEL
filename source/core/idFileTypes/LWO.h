#pragma once

#include <string>
#include <vector>
#include <cmath>

#pragma pack(push)	// Not portable, sorry.
#pragma pack(1)		// Works on my machine (TM).

namespace HAYDEN
{
    struct LWO_VERTEX_PACKED
    {
	uint16_t X = 0;
	uint16_t Y = 0;
	uint16_t Z = 0;
	uint16_t NullPad = 0;
    };

    struct LWO_VERTEX
    {	
	float_t X = 0;
	float_t Y = 0;
	float_t Z = 0;
    };

    struct LWO_NORMAL_PACKED
    {
	uint8_t Xn = 0;
	uint8_t Yn = 0;
	uint8_t Zn = 0;
	uint8_t Always0 = 0;
	uint8_t Xt = 0;
	uint8_t Yt = 0;
	uint8_t Zt = 0;
	uint8_t Always128 = 0;
    };

    struct LWO_UV_PACKED
    {
	uint16_t U = 0;
	uint16_t V = 0;
    };

    struct LWO_FACE_GROUP
    {
	uint16_t F1 = 0;
	uint16_t F2 = 0;
	uint16_t F3 = 0;
    };

    struct LWO_NORMAL
    {
	float_t Xn = 0;
	float_t Yn = 0;
	float_t Zn = 0;
    };

    struct LWO_UV
    {
	float_t U = 0;
	float_t V = 0;
    };

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

    struct LWO_MESH_FOOTER
    {
        uint32_t UnkInt3 = 0;
        uint32_t Dummy1 = 0;
        uint32_t NullPad = 0;
    };

    struct LWO_BML_HEADER
    {
	uint32_t NullPad32 = 0;
	uint32_t DummyMask = 0;
	uint32_t NumVertices = 0;
	uint32_t NumFacesX3 = 0;
        uint16_t LWOVersion = 0;                // indicates model type. 60 is standard. 124 = UV Lightmap
        uint16_t LWOVersion2 = 0;               // always 2 (?)

	float_t NegBoundsX = 0;
	float_t NegBoundsY = 0;
	float_t NegBoundsZ = 0;
	float_t PosBoundsX = 0;
	float_t PosBoundsY = 0;
	float_t PosBoundsZ = 0;

	float_t VertexOffsetX = 0;
	float_t VertexOffsetY = 0;
	float_t VertexOffsetZ = 0;
	float_t VertexScale = 0;

	float_t UVMapOffsetU = 0;
	float_t UVMapOffsetV = 0;
	float_t UVScale = 0;

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
        std::vector<LWO_BML_HEADER> BMLHeaders;
        uint32_t UnkTuple[2] = { 0 };              // Only if LWO_HEADER.UnkHash == 0
    };

    struct LWO_MESH
    {
        uint32_t UnkInt1 = 0;
        uint32_t UnkInt2 = 0;
        uint32_t DeclStrlen = 0;
        std::string Material2DeclName;
        uint32_t UnkInt3 = 0;
        uint32_t Dummy1 = 0;
        uint32_t NullPad = 0;
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
        uint32_t LOD_NormalStartOffset = 0;
        uint32_t LOD_UVStartOffset = 0;
        uint32_t LOD_ColorStartOffset = 0;
        uint32_t LOD_FacesStartOffset = 0;
    };

    struct LWO_STREAMDB_DATA_VARIANT
    {
        uint32_t UnkNormalInt = 32;
        uint32_t UnkUVInt = 20;
        uint32_t UnkUVLightmapInt = 64;             // 124 variant only
        uint32_t UnkColorInt = 131072;
        uint32_t UnkFacesInt = 8;
        uint32_t UnkInt99 = 0;
        uint32_t LOD_NormalStartOffset = 0;
        uint32_t LOD_UVStartOffset = 0;
        uint32_t LOD_UVLightMapOffset = 0;          // 124 variant only
        uint32_t LOD_ColorStartOffset = 0;
        uint32_t LOD_FacesStartOffset = 0;
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
            std::vector<LWO_STREAMDB_HEADER> LWOStreamDBHeaders;
            std::vector<LWO_STREAMDB_DATA> LWOStreamDBData;                         // Used if LWO version = 60
            std::vector<LWO_STREAMDB_DATA_VARIANT> LWOStreamDBDataVariant;          // Used in LWO version = 124 (uv lightmap)
            std::vector<LWO_GEOMETRY_STREAMDISK_LAYOUT> LWOGeoStreamDiskLayout;

        private:

            // Defaults
            int  _BMLCount = 3;
            bool _UseExtendedBML = 0;
            uint32_t _Num32ByteChunks = 0;
    };

    // Extracted from *.streamdb files
    class LWO_GEO
    {
	public:
	    std::vector<LWO_VERTEX> Vertices;
	    std::vector<LWO_NORMAL> Normals;
	    std::vector<LWO_UV> UVs;
	    std::vector<LWO_FACE_GROUP> Faces;
	    LWO_VERTEX UnpackVertex(LWO_VERTEX_PACKED vertexPacked, float_t offsetX, float_t offsetY, float_t offsetZ, float_t vertexScale);
	    LWO_NORMAL UnpackNormal(LWO_NORMAL_PACKED normalPacked);
	    LWO_UV UnpackUV(LWO_UV_PACKED uvPacked, float_t offsetU, float_t offsetV, float_t uvScale);
    };

    class LWO
    {
	public:
            LWO_HEADER Header;
            std::vector<LWO_GEO> Geo;
	    void Serialize(LWO_HEADER lwoHeader, std::vector<uint8_t> lwoGeo);
    };
}

#pragma pack(pop)