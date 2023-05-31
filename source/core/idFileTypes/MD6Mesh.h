#pragma once

#include <string>
#include <vector>
#include <cmath>

#include "../Common.h"
#include "StreamDBGeometry.h"
#include "../ResourceManager.h"


namespace HAYDEN {
    HAYDEN_PACK(
            struct MD6_UNK_FLOATS {
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
            }
    )
    HAYDEN_PACK(
            struct MD6_BONE_INFO {
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
                uint8_t unkAlwaysNull[48] = {0};
            }
    )
    HAYDEN_PACK(
            struct MD6_MESH_UNKNOWNS {
                uint8_t UnkChar1 = 0;
                uint32_t UnkDummy1 = 0xFFFFFFFF;
                int32_t UnkInt1 = 0;
                uint32_t UnkInt2 = 0;
                uint32_t UnkInt3 = 0;
                uint32_t UnkInt4 = 0;
            }
    )
    HAYDEN_PACK(
            struct MD6LodInfo {
                uint32_t NullPad32 = 0;
                uint32_t NumVertices = 0;
                uint32_t NumFaces = 0;

                GeoMetadata Meta;
                GeoFlags Flags;

                float_t UnkFloat1 = 0;
                float_t UnkFloat2 = 0;
            }
    )
    HAYDEN_PACK(
            struct MD6MeshFooter {
                uint32_t UnkInt1 = 0;
                uint32_t UnkInt2 = 0;
                uint32_t UnkInt3 = 0;
                uint8_t UnkChar1 = 0;
            }
    )
    HAYDEN_PACK(
            struct MD6MeshInfo {
                uint32_t MeshNameStrlen = 0;
                std::string MeshName;
                uint32_t MaterialDeclStrlen = 0;
                std::string MaterialDeclName;
                MD6_MESH_UNKNOWNS MeshUnknowns;
                std::vector<MD6LodInfo> LODInfo;
                MD6MeshFooter MeshFooter;
                uint32_t UnkTuple[2] = {0};               // Only if MD6_BONE_INFO.UnkFloat7 and UnkFloat8 == 0
            }
    )
    HAYDEN_PACK(
            struct MD6StreamDBHeader {
                uint16_t MD6Version = 60;                   // 60 is normal, 124 is uvlayout_lightmap = 1
                uint16_t MD6Version2 = 2;                   // always 2?
                uint32_t DecompressedSize = 0;
                uint32_t NumOffsets = 4;                    // always 4 (or 5 if uvlayout_lightmap = 1)
            }
    )
    HAYDEN_PACK(
            struct MD6StreamDBData {
                uint32_t UnkNormalInt = 32;
                uint32_t UnkUVInt = 20;
                uint32_t UnkColorInt = 131072;
                uint32_t UnkFacesInt = 8;
                uint32_t UnkInt99 = 0;
                uint32_t NormalStartOffset = 0;
                uint32_t UVStartOffset = 0;
                uint32_t BoneIdStartOffset = 0;
                uint32_t FacesStartOffset = 0;
            }
    )
    HAYDEN_PACK(
            struct MD6StreamDBDataVariant {
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
            }
    )
    HAYDEN_PACK(
            struct MD6GeometryStreamDiskLayout {
                uint32_t StreamCompressionType = 3;         // 3 = NONE_MODEL, 4 = KRAKEN_MODEL
                uint32_t DecompressedSize = 0;
                uint32_t CompressedSize = 0;
                uint32_t CumulativeStreamDBCompSize = 0;
            }
    )

    // Extracted from *.resources files
    class MD6MeshHeader {
    public:

        std::string m_sklFilename;
        MD6_UNK_FLOATS m_unkFloats;

        uint16_t m_boneCount = 0;
        std::vector<uint8_t> m_boneIds;
        MD6_BONE_INFO m_boneInfo;

        uint32_t m_meshCount = 0;
        std::vector<MD6MeshInfo> m_meshInfo;

        uint32_t m_streamCount = 5;
        std::vector<MD6StreamDBHeader> m_streamDBHeaders;
        std::vector<MD6StreamDBData> m_streamDBData;
        std::vector<MD6StreamDBDataVariant> m_streamDBDataVariant;
        std::vector<MD6GeometryStreamDiskLayout> m_streamDiskLayout;

        std::vector<uint8_t> m_rawMD6Header; // Temporary for rev-eng
        void fromData(const std::vector<uint8_t> &binaryData);
    };

    class MD6Mesh {
    public:
        MD6Mesh(const ResourceManager &resourceManager, const std::string &resourcePath);

        void readStreamData(const std::vector<uint8_t>& md6Geo);

        [[nodiscard]] bool loaded() const { return m_loaded; }

        [[nodiscard]] const MD6MeshHeader &header() const { return m_header; }

        [[nodiscard]] const std::vector<Mesh> &meshGeometry() const { return m_meshGeometry; }

    private:
        bool m_loaded;
        MD6MeshHeader m_header;
        std::vector<Mesh> m_meshGeometry;
    };
}