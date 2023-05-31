#include "LWO.h"
#include "source/core/Oodle.h"

namespace HAYDEN {
    void LWOHeader::fromData(const std::vector<uint8_t> &binaryData) {
        int offset = 0;

        // Read number of meshes
        m_metadata = *(LWOMetadata *) (binaryData.data() + offset);
        m_meshInfo.resize(m_metadata.NumMeshes);

        // Determine BMLr type & count
        if (m_metadata.UnkHash == 0) {
            m_bblCount = 1;
            m_useExtendedBML = true;
        } else if (m_metadata.UnkHash != 0 && m_metadata.NullPad32_2 != 0) {
            m_bblCount = 4;
            m_useExtendedBML = false;
            m_useShortMeshHeader = true;
        } else {
            m_bblCount = 3;
            m_useExtendedBML = false;
        }

        // if (m_metadata.NullPad32_2 != 0)
        // {
        // ADD ERROR HANDLER
        //    return;
        // }

        // Advance offset to start of Mesh info
        offset += sizeof(LWOMetadata);

        // Read mesh and material info
        for (int i = 0; i < m_metadata.NumMeshes; i++) {
            if (i > 0 && m_useShortMeshHeader) {
                m_meshInfo[i].MeshHeader.UnkInt2 = *(uint32_t *) (binaryData.data() + offset);
                offset += sizeof(uint32_t);
                m_meshInfo[i].MeshHeader.DeclStrlen = *(uint32_t *) (binaryData.data() + offset);
                offset += sizeof(uint32_t);
            } else {
                m_meshInfo[i].MeshHeader = *(LWO_MESH_HEADER *) (binaryData.data() + offset);
                offset += sizeof(LWO_MESH_HEADER);
            }

            uint32_t strLen = m_meshInfo[i].MeshHeader.DeclStrlen;

            if (strLen > 1024) {
                // ADD ERROR HANDLER
                return;
            }

            m_meshInfo[i].MaterialDeclName.resize(strLen);
            m_meshInfo[i].MaterialDeclName = std::string(binaryData.data() + offset,
                                                         binaryData.data() + offset + strLen);
            offset += strLen;

            m_meshInfo[i].MeshFooter = *(LWO_MESH_FOOTER *) (binaryData.data() + offset);
            offset += sizeof(LWO_MESH_FOOTER);

            // Read BMLr data (level-of-detail info for each mesh)
            m_meshInfo[i].LODInfo.resize(m_bblCount);

            for (int j = 0; j < m_bblCount; j++) {
                m_meshInfo[i].LODInfo[j] = *(LWOLodInfo *) (binaryData.data() + offset);

                // test
                if (m_meshInfo[i].LODInfo[j].NullPad32 != 0) {
                    m_bblCount = j;
                    m_meshInfo[i].LODInfo.resize(m_bblCount);
                    offset += sizeof(uint32_t);
                } else {
                    offset += sizeof(LWOLodInfo);
                }

                // No idea what these are for
                if (m_useExtendedBML) {
                    m_meshInfo[i].UnkTuple[0] = *(uint32_t *) (binaryData.data() + offset);
                    offset += sizeof(uint32_t);

                    m_meshInfo[i].UnkTuple[1] = *(uint32_t *) (binaryData.data() + offset);
                    offset += sizeof(uint32_t);
                }
            }
        }

        // Everything above has been read sequentially.
        // But now, we will skip a bunch of data in the middle of the LWO header.
        // The size varies considerably, and most of this data has no known purpose. It is not needed for exporting to OBJ
        // The last thing we need for exporting is StreamDB info, which is located at the END of the file.

        // StreamDB structure depends on what our LWO version is.
        const uint64_t streamDBStructCount = 5;
        m_streamDBHeaders.resize(streamDBStructCount);
        m_streamDiskLayout.resize(streamDBStructCount);
        uint64_t streamDBStructSize = sizeof(LWOStreamdbHeader) + sizeof(LWOGeometryStreamdiskLayout);

        if (m_meshInfo[0].LODInfo[0].GeoFlags.Flags1 == 60) {
            streamDBStructSize += sizeof(LWOStreamdbData);
            m_streamDBData.resize(streamDBStructCount);
        } else {
            streamDBStructSize += sizeof(LWOStreamdbDataVariant);
            m_streamDBDataVariant.resize(streamDBStructCount);
        }

        // Move offset to the beginning of streamDB structures
        // Calculated from end of file.
        offset = binaryData.size() - (streamDBStructSize * streamDBStructCount);

        // Read StreamDB info
        for (int i = 0; i < streamDBStructCount; i++) {
            m_streamDBHeaders[i] = *(LWOStreamdbHeader *) (binaryData.data() + offset);
            offset += sizeof(LWOStreamdbHeader);

            if (m_meshInfo[0].LODInfo[0].GeoFlags.Flags1 == 60) {
                m_streamDBData[i] = *(LWOStreamdbData *) (binaryData.data() + offset);
                offset += sizeof(LWOStreamdbData);
            } else {
                m_streamDBDataVariant[i] = *(LWOStreamdbDataVariant *) (binaryData.data() + offset);
                offset += sizeof(LWOStreamdbDataVariant);
            }

            m_streamDiskLayout[i] = *(LWOGeometryStreamdiskLayout *) (binaryData.data() + offset);
            offset += sizeof(LWOGeometryStreamdiskLayout);
        }

        // Temporary for rev-eng
        m_rawLWOHeader.insert(m_rawLWOHeader.begin(), binaryData.begin(), binaryData.end());
        return;
    }

    LWO::LWO(const ResourceManager &resourceManager, const std::string &resourcePath) {
        m_loaded = false;
        auto headerData = resourceManager.queryFileByName(resourcePath);
        if (!headerData.has_value()) {
            fprintf(stderr, "Error: No LWO header data for %s\n", resourcePath.c_str());
            return;
        }
        m_header.fromData(headerData.value());
        m_meshGeometry.resize(m_header.m_metadata.NumMeshes);

        auto optStreamData = resourceManager.queryStreamDataByName(resourcePath,
                                                                   m_header.m_streamDiskLayout[0].CompressedSize);
        if (!optStreamData.has_value()) {
            fprintf(stderr, "Error: No LWO stream data for %s\n", resourcePath.c_str());
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

    void LWO::readStreamData(const std::vector<uint8_t> &data) {

        uint64_t offset = 0;
        uint64_t offsetNormals;
        uint64_t offsetUVs;
        uint64_t offsetFaces;

        // Get offsets from LWO streamdb data
        if (m_header.m_meshInfo[0].LODInfo[0].GeoFlags.Flags1 == 60) {
            offsetNormals = m_header.m_streamDBData[0].NormalStartOffset;
            offsetUVs = m_header.m_streamDBData[0].UVStartOffset;
            offsetFaces = m_header.m_streamDBData[0].FacesStartOffset;
        } else {
            offsetNormals = m_header.m_streamDBDataVariant[0].NormalStartOffset;
            offsetUVs = m_header.m_streamDBDataVariant[0].UVStartOffset;
            offsetFaces = m_header.m_streamDBDataVariant[0].FacesStartOffset;
        }

        // Allocate
        for (int i = 0; i < m_meshGeometry.size(); i++) {
            m_meshGeometry[i].m_vertices.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_normals.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_uv.resize(m_header.m_meshInfo[i].LODInfo[0].NumVertices);
            m_meshGeometry[i].m_faces.resize(m_header.m_meshInfo[i].LODInfo[0].NumEdges / 3);
        }

        // Read m_vertices
        for (int i = 0; i < m_meshGeometry.size(); i++) {
            for (int j = 0; j < m_meshGeometry[i].m_vertices.size(); j++) {
                m_meshGeometry[i].m_vertices[j] = Mesh::UnpackVertex(*(PackedVertex *) (data.data() + offset),
                                                                     m_header.m_meshInfo[i].LODInfo[0].GeoMeta.VertexOffset,
                                                                     m_header.m_meshInfo[i].LODInfo[0].GeoMeta.VertexScale);
                offset += sizeof(PackedVertex);
            }
        }

        // Read m_normals
        offset = offsetNormals;
        for (auto &meshGeometry: m_meshGeometry) {
            for (auto &m_normal: meshGeometry.m_normals) {
                m_normal = Mesh::UnpackNormal(*(PackedNormal *) (data.data() + offset));
                offset += sizeof(PackedNormal);
            }
        }

        // Read m_uv
        offset = offsetUVs;
        for (int i = 0; i < m_meshGeometry.size(); i++) {
            for (int j = 0; j < m_meshGeometry[i].m_uv.size(); j++) {
                m_meshGeometry[i].m_uv[j] = Mesh::UnpackUV(*(PackedUV *) (data.data() + offset),
                                                           m_header.m_meshInfo[i].LODInfo[0].GeoMeta.UVMapOffset,
                                                           m_header.m_meshInfo[i].LODInfo[0].GeoMeta.UVScale);
                offset += sizeof(PackedUV);
            }
        }

        // Read m_faces
        offset = offsetFaces;
        for (auto &meshGeometry: m_meshGeometry) {
            std::memcpy(meshGeometry.m_faces.data(), data.data() + offset, meshGeometry.m_faces.size() * sizeof(Face));
        }
    }
}