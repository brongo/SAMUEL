#pragma once

#include <string>
#include <vector>
#include <cmath>

#include "../Common.h"
#include "StreamDBGeometry.h"
#include "../ResourceManager.h"

namespace HAYDEN {
    class LWO;

    HAYDEN_PACK(
            struct LWOMetadata {
                uint64_t NullPad64_0 = 0;
                uint32_t NullPad32_0 = 0;
                uint32_t FileType = 0;
                uint32_t NumMeshes = 0;
                uint32_t NullPad32_1 = 0;
                uint64_t UnkHash = 0;                // if zero, uses UnkTuple[2] at end of BML headers
                uint32_t NullPad32_2 = 0;
            })
    HAYDEN_PACK(
            struct LWO_MESH_HEADER {
                uint32_t UnkInt1 = 0;
                uint32_t UnkInt2 = 0;
                uint32_t DeclStrlen = 0;
            })
    HAYDEN_PACK(
            struct LWO_MESH_HEADER_SHORT {
                uint32_t UnkInt2 = 0;
                uint32_t DeclStrlen = 0;
            })
    HAYDEN_PACK(
            struct LWO_MESH_FOOTER {
                uint32_t UnkInt3 = 0;
                uint32_t Dummy1 = 0;
                uint32_t NullPad = 0;
            })
    HAYDEN_PACK(
            struct LWOLodInfo {
                uint32_t NullPad32 = 0;
                uint32_t DummyMask = 0;
                uint32_t NumVertices = 0;
                uint32_t NumEdges = 0;

                GeoFlags GeoFlags;
                GeoMetadata GeoMeta;

                float_t UnkFloat1 = 0;
                float_t UnkFloat2 = 0;
                float_t UnkFloat3 = 0;
                char Signature[4] = {0};
            })
    HAYDEN_PACK(
            struct LWOMeshInfo {
                LWO_MESH_HEADER MeshHeader;
                std::string MaterialDeclName;
                LWO_MESH_FOOTER MeshFooter;
                std::vector<LWOLodInfo> LODInfo;
                uint32_t UnkTuple[2] = {0};              // Only if LWOHeader.UnkHash == 0
            })
    HAYDEN_PACK(
            struct LWOStreamdbHeader {
                uint32_t NumStreams = 1;
                uint16_t LWOVersion = 60;                   // 60 is normal, 124 is uvlayout_lightmap = 1
                uint16_t LWOVersion2 = 2;
                uint32_t DecompressedSize = 0;
                uint32_t NumOffsets = 4;                    // always 4 (or 5 if uvlayout_lightmap = 1)
            })
    HAYDEN_PACK(
            struct LWOStreamdbData {
                uint32_t UnkNormalInt = 32;
                uint32_t UnkUVInt = 20;
                uint32_t UnkColorInt = 131072;
                uint32_t UnkFacesInt = 8;
                uint32_t UnkInt99 = 0;
                uint32_t NormalStartOffset = 0;
                uint32_t UVStartOffset = 0;
                uint32_t ColorStartOffset = 0;
                uint32_t FacesStartOffset = 0;
            })
    HAYDEN_PACK(
            struct LWOStreamdbDataVariant {
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
            })
    HAYDEN_PACK(
            struct LWOGeometryStreamdiskLayout {
                uint32_t StreamCompressionType = 3;         // 3 = NONE_MODEL, 4 = KRAKEN_MODEL
                uint32_t DecompressedSize = 0;
                uint32_t CompressedSize = 0;
                uint32_t CumulativeStreamDBCompSize = 0;
            })


    // Extracted from *.resources files
    class LWOHeader {
    public:

        void fromData(const std::vector<uint8_t> &binaryData);

        // Serialized file data
        LWOMetadata m_metadata;

        std::vector<LWOMeshInfo> m_meshInfo;
        std::vector<LWOStreamdbHeader> m_streamDBHeaders;
        std::vector<LWOStreamdbData> m_streamDBData;                         // Used if LWO version = 60
        std::vector<LWOStreamdbDataVariant> m_streamDBDataVariant;          // Used in LWO version = 124 (uv lightmap)
        std::vector<LWOGeometryStreamdiskLayout> m_streamDiskLayout;
        std::vector<uint8_t> m_rawLWOHeader;

        // Defaults
        int m_bblCount = 3;
        bool m_useExtendedBML = false;
        bool m_useShortMeshHeader = false;
        uint32_t m_num32ByteChunks = 0;

        friend LWO;
    };

    class LWO {
    public:
        LWO(const ResourceManager &resourceManager, const std::string &resourcePath);

        void readStreamData(const std::vector<uint8_t>& data);


        [[nodiscard]] bool loaded() const { return m_loaded; }

        [[nodiscard]] const LWOHeader &header() const { return m_header; }

        [[nodiscard]] const std::vector<Mesh> &meshGeometry() const { return m_meshGeometry; }

    private:

        bool m_loaded;
        LWOHeader m_header;
        std::vector<Mesh> m_meshGeometry;
    };
}