#include "MD6Mesh.h"
#include "../ResourceManager.h"

namespace HAYDEN {
    void MD6MeshHeader::fromData(const std::vector<uint8_t> &binaryData) {
        size_t offset = 0;

        // Temporary for rev-eng
        m_rawMD6Header.insert(m_rawMD6Header.begin(), binaryData.begin(), binaryData.end());

        // Read header
        uint32_t skeletonNameSize = *(uint32_t *) (binaryData.data() + offset);
        offset += sizeof(uint32_t);

        m_sklFilename = std::string(binaryData.data() + offset, binaryData.data() + offset + skeletonNameSize);
        offset += skeletonNameSize;

        m_unkFloats = *(MD6_UNK_FLOATS *) (binaryData.data() + offset);
        offset += sizeof(MD6_UNK_FLOATS);

        // Read bone info
        m_boneCount = *(uint16_t *) (binaryData.data() + offset);
        m_boneIds.resize(m_boneCount);
        offset += sizeof(uint16_t);

        for (int i = 0; i < m_boneCount; i++) {
            m_boneIds[i] = *(uint8_t *) (binaryData.data() + offset);
            offset += sizeof(uint8_t);
        }

        m_boneInfo = *(MD6_BONE_INFO *) (binaryData.data() + offset);
        offset += sizeof(MD6_BONE_INFO);

        // Read mesh info
        m_meshCount = *(uint32_t *) (binaryData.data() + offset);
        m_meshInfo.resize(m_meshCount);
        offset += sizeof(uint32_t);

        for (int i = 0; i < m_meshCount; i++) {
            m_meshInfo[i].MeshNameStrlen = *(uint32_t *) (binaryData.data() + offset);
            offset += sizeof(uint32_t);

            m_meshInfo[i].MeshName.resize(m_meshInfo[i].MeshNameStrlen);
            m_meshInfo[i].MeshName = std::string(binaryData.data() + offset,
                                                 binaryData.data() + offset + m_meshInfo[i].MeshNameStrlen);
            offset += m_meshInfo[i].MeshNameStrlen;

            // Read Material2 .decl name
            m_meshInfo[i].MaterialDeclStrlen = *(uint32_t *) (binaryData.data() + offset);
            offset += sizeof(uint32_t);

            m_meshInfo[i].MaterialDeclName.resize(m_meshInfo[i].MaterialDeclStrlen);
            m_meshInfo[i].MaterialDeclName = std::string(binaryData.data() + offset,
                                                         binaryData.data() + offset + m_meshInfo[i].MaterialDeclStrlen);
            offset += m_meshInfo[i].MaterialDeclStrlen;

            // Read MD6Mesh unknown data
            m_meshInfo[i].MeshUnknowns = *(MD6_MESH_UNKNOWNS *) (binaryData.data() + offset);
            offset += sizeof(MD6_MESH_UNKNOWNS);

            // Read LODInfo - very similar to LWO BMLr stuff
            m_meshInfo[i].LODInfo.resize(3);

            if (m_boneInfo.UnkFloat7 == 0 && m_boneInfo.UnkFloat8 == 0) {
                // Short meshinfo variant
                for (int j = 0; j < 1; j++) {
                    m_meshInfo[i].LODInfo[j] = *(MD6LodInfo *) (binaryData.data() + offset);
                    offset += sizeof(MD6LodInfo);

                    m_meshInfo[i].UnkTuple[0] = *(uint32_t *) (binaryData.data() + offset);
                    offset += sizeof(uint32_t);

                    m_meshInfo[i].UnkTuple[1] = *(uint32_t *) (binaryData.data() + offset);
                    offset += sizeof(uint32_t);
                }
            } else {
                // Standard meshinfo 
                for (int j = 0; j < 3; j++) {
                    m_meshInfo[i].LODInfo[j] = *(MD6LodInfo *) (binaryData.data() + offset);
                    offset += sizeof(MD6LodInfo);
                }
            }

            // Read footer unknowns
            m_meshInfo[i].MeshFooter = *(MD6MeshFooter *) (binaryData.data() + offset);
            offset += sizeof(MD6MeshFooter);
        }

        // StreamDB structure depends on what our MD6Mesh version is.
        const uint64_t streamDBStructCount = 5;
        m_streamDBHeaders.resize(streamDBStructCount);
        m_streamDiskLayout.resize(streamDBStructCount);
        uint64_t streamDBStructSize = sizeof(MD6StreamDBHeader) + sizeof(MD6GeometryStreamDiskLayout);

        // Assume we are dealing with version 60 for now - no variants
        streamDBStructSize += sizeof(MD6StreamDBData);
        m_streamDBData.resize(streamDBStructCount);

        // Move offset to the beginning of streamDB structures
        // Calculated from end of file.
        offset = binaryData.size() - (streamDBStructSize * streamDBStructCount);

        // Move back another 4 bytes to get m_streamCount
        offset = offset - 4;

        // Read m_streamCount
        m_streamCount = *(uint32_t *) (binaryData.data() + offset);
        offset += 4;

        if (m_streamCount != 5)
            return; // abort

        // Read StreamDB info
        for (int i = 0; i < streamDBStructCount; i++) {
            m_streamDBHeaders[i] = *(MD6StreamDBHeader *) (binaryData.data() + offset);
            offset += sizeof(MD6StreamDBHeader);

            m_streamDBData[i] = *(MD6StreamDBData *) (binaryData.data() + offset);
            offset += sizeof(MD6StreamDBData);
        }

        // Read GeoStreamDiskLayouts
        for (int i = 0; i < streamDBStructCount; i++) {
            m_streamDiskLayout[i] = *(MD6GeometryStreamDiskLayout *) (binaryData.data() + offset);
            offset += sizeof(MD6GeometryStreamDiskLayout);
        }

    }

    void MD6Mesh::readStreamData(std::vector<uint8_t> md6Geo) {
        m_meshGeometry.resize(m_header.m_meshCount);

        uint64_t offset = 0;
        uint64_t offsetNormals;
        uint64_t offsetUVs;
        uint64_t offsetBoneIds;
        uint64_t offsetFaces;

        // Get offsets from MD6Mesh streamdb data
        offsetNormals = m_header.m_streamDBData[0].NormalStartOffset;
        offsetUVs = m_header.m_streamDBData[0].UVStartOffset;
        offsetFaces = m_header.m_streamDBData[0].FacesStartOffset;
        offsetBoneIds = m_header.m_streamDBData[0].BoneIdStartOffset;

        // Allocate
        for (int i = 0; i < m_meshGeometry.size(); i++) {
            m_meshGeometry[i].m_vertices.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_normals.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_boneIds.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_boneWeights.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_uv.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_faces.resize(m_header.m_meshInfo[i].LODInfo[0].NumFaces);
        }

        // Read m_vertices
        for (int i = 0; i < m_meshGeometry.size(); i++) {
            for (int j = 0; j < m_meshGeometry[i].m_vertices.size(); j++) {
                PackedVertex packedVertex = *(PackedVertex *) (md6Geo.data() + offset);
                m_meshGeometry[i].m_vertices[j] = m_meshGeometry[i].UnpackVertex(packedVertex,
                                                                                 m_header.m_meshInfo[i].LODInfo[0].Meta.VertexOffset,
                                                                                 m_header.m_meshInfo[i].LODInfo[0].Meta.VertexScale);
                offset += sizeof(PackedVertex);
            }
        }

        // Read m_normals
        offset = offsetNormals;
        const float divisor1 = 1.0f / 254;
        const float divisor2 = 1.0f / 45;
        const float divisor3 = 1.0f / 60;

        for (auto &subMesh: m_meshGeometry) {
            for (int j = 0; j < subMesh.m_normals.size(); j++) {
                PackedNormal packedNormal = *(PackedNormal *) (md6Geo.data() + offset);
                uint8_t pw1 = packedNormal.weights1;
                uint8_t pw2 = packedNormal.weights2 & 0x7F;
                float w2 = static_cast<float>(pw2) * divisor1;
                float w3 = static_cast<float>(pw1 >> 4) * divisor2;
                float w4 = static_cast<float>(pw1 & 0xF) * divisor3;
                float w1 = 1.0f - w2 - w3 - w4;
                subMesh.m_boneWeights[j] = {w1, w2, w3, w4};
                subMesh.m_normals[j] = HAYDEN::Mesh::UnpackNormal(packedNormal);
                offset += sizeof(PackedNormal);
            }
        }

        // Read m_boneIds
        offset = offsetBoneIds;
        for (auto &subMesh: m_meshGeometry) {
            std::memcpy(subMesh.m_boneIds.data(), md6Geo.data() + offset, sizeof(BoneIds) * subMesh.m_boneIds.size());
            offset += sizeof(BoneIds) * subMesh.m_boneIds.size();
        }

        // Read m_uv
        offset = offsetUVs;
        for (int subMeshId = 0; subMeshId < m_meshGeometry.size(); subMeshId++) {
            Mesh &subMesh = m_meshGeometry[subMeshId];
            for (auto &UV: subMesh.m_uv) {
                PackedUV packedUV = *(PackedUV *) (md6Geo.data() + offset);
                GeoMetadata &subMeshLodMeta = m_header.m_meshInfo[subMeshId].LODInfo[0].Meta;
                UV = HAYDEN::Mesh::UnpackUV(packedUV,
                                            subMeshLodMeta.UVMapOffset,
                                            subMeshLodMeta.UVScale);
                offset += sizeof(PackedUV);
            }
        }

        // Read m_faces
        offset = offsetFaces;
        for (auto &subMesh: m_meshGeometry) {
            std::memcpy(subMesh.m_faces.data(), md6Geo.data() + offset, sizeof(Face) * subMesh.m_faces.size());
            offset += sizeof(Face) * subMesh.m_faces.size();
        }
    }

    MD6Mesh::MD6Mesh(const ResourceManager &resourceManager, const std::string &resourcePath) {
        m_loaded = false;
        auto headerData = resourceManager.queryFileByName(resourcePath);
        if (!headerData.has_value()) {
            return;
        }

        m_header.fromData(headerData.value());

        auto optStreamData = resourceManager.queryStreamDataByName(resourcePath,
                                                                   m_header.m_streamDiskLayout[0].CompressedSize);
        if (!optStreamData.has_value()) {
            return;
        }
        std::vector<uint8_t> streamData = optStreamData.value();
        if (m_header.m_streamDiskLayout[0].CompressedSize != m_header.m_streamDiskLayout[0].DecompressedSize) {
            oodleDecompressInplace(streamData, m_header.m_streamDiskLayout[0].DecompressedSize);
            if (streamData.empty()) {
                fprintf(stderr, "Error: Failed to decompress: %s \n", resourcePath.c_str());
                return;
            }
        }

        m_loaded = true;
        readStreamData(streamData);
    }
}