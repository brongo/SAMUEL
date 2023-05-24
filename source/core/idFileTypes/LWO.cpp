#include "LWO.h"

namespace HAYDEN
{
    void LWO_HEADER::ReadBinaryHeader(const std::vector<uint8_t> binaryData)
    {
        int offset = 0;

        // Read number of meshes
        Metadata = *(LWO_METADATA*)(binaryData.data() + offset);
        MeshInfo.resize(Metadata.NumMeshes);

        // Determine BMLr type & count
        if (Metadata.UnkHash == 0)
        {
            _BMLCount = 1;
            _UseExtendedBML = 1;
        }
        else if(Metadata.UnkHash != 0 && Metadata.NullPad32_2 != 0)
        {
            _BMLCount = 4;
            _UseExtendedBML = 0;
            _UseShortMeshHeader = 1;
        }
        else
        {
            _BMLCount = 3;
            _UseExtendedBML = 0;
        }

        // if (Metadata.NullPad32_2 != 0)
        // {
            // ADD ERROR HANDLER
        //    return;
        // }

        // Advance offset to start of Mesh info
        offset += sizeof(LWO_METADATA);

        // Read mesh and material info
        for (int i = 0; i < Metadata.NumMeshes; i++)
        {
            if (i > 0 && _UseShortMeshHeader)
            {
                MeshInfo[i].MeshHeader.UnkInt2 = *(uint32_t*)(binaryData.data() + offset);
                offset += sizeof(uint32_t);
                MeshInfo[i].MeshHeader.DeclStrlen = *(uint32_t*)(binaryData.data() + offset);
                offset += sizeof(uint32_t);
            }
            else
            {
                MeshInfo[i].MeshHeader = *(LWO_MESH_HEADER*)(binaryData.data() + offset);
                offset += sizeof(LWO_MESH_HEADER);
            }

            int strLen = MeshInfo[i].MeshHeader.DeclStrlen;

            if (strLen < 0 || strLen > 1024)
            {
                // ADD ERROR HANDLER
                return;
            }

            MeshInfo[i].MaterialDeclName.resize(strLen);
            MeshInfo[i].MaterialDeclName = std::string(binaryData.data() + offset, binaryData.data() + offset + strLen);
            offset += strLen;

            MeshInfo[i].MeshFooter = *(LWO_MESH_FOOTER*)(binaryData.data() + offset);
            offset += sizeof(LWO_MESH_FOOTER);

            // Read BMLr data (level-of-detail info for each mesh)
            MeshInfo[i].LODInfo.resize(_BMLCount);

            for (int j = 0; j < _BMLCount; j++)
            {
                MeshInfo[i].LODInfo[j] = *(LWO_LOD_INFO*)(binaryData.data() + offset);

                // test
                if (MeshInfo[i].LODInfo[j].NullPad32 != 0)
                {
                    _BMLCount = j;
                    MeshInfo[i].LODInfo.resize(_BMLCount);
                    offset += sizeof(uint32_t);
                }
                else
                {
                    offset += sizeof(LWO_LOD_INFO);
                }

                // No idea what these are for
                if (_UseExtendedBML)
                {
                    MeshInfo[i].UnkTuple[0] = *(uint32_t*)(binaryData.data() + offset);
                    offset += sizeof(uint32_t);

                    MeshInfo[i].UnkTuple[1] = *(uint32_t*)(binaryData.data() + offset);
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
        StreamDBHeaders.resize(streamDBStructCount);
        StreamDiskLayout.resize(streamDBStructCount);
        uint64_t streamDBStructSize = sizeof(LWO_STREAMDB_HEADER) + sizeof(LWO_GEOMETRY_STREAMDISK_LAYOUT);

        if (MeshInfo[0].LODInfo[0].GeoFlags.Flags1 == 60)
        {
            streamDBStructSize += sizeof(LWO_STREAMDB_DATA);
            StreamDBData.resize(streamDBStructCount);
        }
        else
        {
            streamDBStructSize += sizeof(LWO_STREAMDB_DATA_VARIANT);
            StreamDBDataVariant.resize(streamDBStructCount);
        }

        // Move offset to the beginning of streamDB structures
        // Calculated from end of file.
        offset = binaryData.size() - (streamDBStructSize * streamDBStructCount);

        // Read StreamDB info
        for (int i = 0; i < streamDBStructCount; i++)
        {
            StreamDBHeaders[i] = *(LWO_STREAMDB_HEADER*)(binaryData.data() + offset);
            offset += sizeof(LWO_STREAMDB_HEADER);

            if (MeshInfo[0].LODInfo[0].GeoFlags.Flags1 == 60)
            {
                StreamDBData[i] = *(LWO_STREAMDB_DATA*)(binaryData.data() + offset);
                offset += sizeof(LWO_STREAMDB_DATA);
            }
            else
            {
                StreamDBDataVariant[i] = *(LWO_STREAMDB_DATA_VARIANT*)(binaryData.data() + offset);
                offset += sizeof(LWO_STREAMDB_DATA_VARIANT);
            }

            StreamDiskLayout[i] = *(LWO_GEOMETRY_STREAMDISK_LAYOUT*)(binaryData.data() + offset);
            offset += sizeof(LWO_GEOMETRY_STREAMDISK_LAYOUT);
        }

        // Temporary for rev-eng
        RawLWOHeader.insert(RawLWOHeader.begin(), binaryData.begin(), binaryData.end());
        return;
    }

    void LWO::Serialize(LWO_HEADER lwoHeader, std::vector<uint8_t> lwoGeo)
    {
        Header = lwoHeader;
        MeshGeometry.resize(Header.Metadata.NumMeshes);

        uint64_t offset = 0;
        uint64_t offsetNormals = 0;
        uint64_t offsetUVs = 0;
        uint64_t offsetFaces = 0;

        // Get offsets from LWO streamdb data
        if (Header.MeshInfo[0].LODInfo[0].GeoFlags.Flags1 == 60)
        {
            offsetNormals = Header.StreamDBData[0].NormalStartOffset;
            offsetUVs = Header.StreamDBData[0].UVStartOffset;
            offsetFaces = Header.StreamDBData[0].FacesStartOffset;
        }
        else
        {
            offsetNormals = Header.StreamDBDataVariant[0].NormalStartOffset;
            offsetUVs = Header.StreamDBDataVariant[0].UVStartOffset;
            offsetFaces = Header.StreamDBDataVariant[0].FacesStartOffset;
        }

        // Allocate
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            MeshGeometry[i].m_vertices.resize(Header.MeshInfo[i].LODInfo[0].NumVertices);
            MeshGeometry[i].m_normals.resize(Header.MeshInfo[i].LODInfo[0].NumVertices);
            MeshGeometry[i].m_uv.resize(Header.MeshInfo[i].LODInfo[0].NumVertices);
            MeshGeometry[i].m_faces.resize(Header.MeshInfo[i].LODInfo[0].NumEdges / 3);
        }

        // Read m_vertices
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].m_vertices.size(); j++)
            {
                PackedVertex packedVertex = *(PackedVertex*)(lwoGeo.data() + offset);
                MeshGeometry[i].m_vertices[j] = MeshGeometry[i].UnpackVertex(packedVertex, Header.MeshInfo[i].LODInfo[0].GeoMeta.VertexOffset, Header.MeshInfo[i].LODInfo[0].GeoMeta.VertexScale);
                offset += sizeof(PackedVertex);
            }
        }
     
        // Read m_normals
        offset = offsetNormals;
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].m_normals.size(); j++)
            {
                PackedNormal packedNormal = *(PackedNormal*)(lwoGeo.data() + offset);
                MeshGeometry[i].m_normals[j] = MeshGeometry[i].UnpackNormal(packedNormal);
                offset += sizeof(PackedNormal);
            }
        }

        // Read m_uv
        offset = offsetUVs;
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].m_uv.size(); j++)
            {
                PackedUV packedUV = *(PackedUV*)(lwoGeo.data() + offset);
                MeshGeometry[i].m_uv[j] = MeshGeometry[i].UnpackUV(packedUV, Header.MeshInfo[i].LODInfo[0].GeoMeta.UVMapOffset, Header.MeshInfo[i].LODInfo[0].GeoMeta.UVScale);
                offset += sizeof(PackedUV);
            }
        }

        // Read m_faces
        offset = offsetFaces;
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].m_faces.size(); j++)
            {
                MeshGeometry[i].m_faces[j] = *(Face*)(lwoGeo.data() + offset);
                offset += sizeof(Face);
            }
        }
        return;
    }
}