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
        else
        {
            _BMLCount = 3;
            _UseExtendedBML = 0;
        }

        if (Metadata.NullPad32_2 != 0)
        {
            // ADD ERROR HANDLER
            return;
        }

        // Advance offset to start of Mesh info
        offset += sizeof(LWO_METADATA);

        // Read mesh and material info
        for (int i = 0; i < Metadata.NumMeshes; i++)
        {
            MeshInfo[i].MeshHeader = *(LWO_MESH_HEADER*)(binaryData.data() + offset);
            offset += sizeof(LWO_MESH_HEADER);

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
            MeshInfo[i].BMLHeaders.resize(_BMLCount);

            for (int j = 0; j < _BMLCount; j++)
            {
                MeshInfo[i].BMLHeaders[j] = *(LWO_BML_HEADER*)(binaryData.data() + offset);
                offset += sizeof(LWO_BML_HEADER);

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
        LWOStreamDBHeaders.resize(streamDBStructCount);
        LWOGeoStreamDiskLayout.resize(streamDBStructCount);
        uint64_t streamDBStructSize = sizeof(LWO_STREAMDB_HEADER) + sizeof(LWO_GEOMETRY_STREAMDISK_LAYOUT);

        if (MeshInfo[0].BMLHeaders[0].LWOVersion == 60)
        {
            streamDBStructSize += sizeof(LWO_STREAMDB_DATA);
            LWOStreamDBData.resize(streamDBStructCount);
        }
        else
        {
            streamDBStructSize += sizeof(LWO_STREAMDB_DATA_VARIANT);
            LWOStreamDBDataVariant.resize(streamDBStructCount);
        }

        // Move offset to the beginning of streamDB structures
        // Calculated from end of file.
        offset = binaryData.size() - (streamDBStructSize * streamDBStructCount);

        // Read StreamDB info
        for (int i = 0; i < streamDBStructCount; i++)
        {
            LWOStreamDBHeaders[i] = *(LWO_STREAMDB_HEADER*)(binaryData.data() + offset);
            offset += sizeof(LWO_STREAMDB_HEADER);

            if (MeshInfo[0].BMLHeaders[0].LWOVersion == 60)
            {
                LWOStreamDBData[i] = *(LWO_STREAMDB_DATA*)(binaryData.data() + offset);
                offset += sizeof(LWO_STREAMDB_DATA);
            }
            else
            {
                LWOStreamDBDataVariant[i] = *(LWO_STREAMDB_DATA_VARIANT*)(binaryData.data() + offset);
                offset += sizeof(LWO_STREAMDB_DATA_VARIANT);
            }

            LWOGeoStreamDiskLayout[i] = *(LWO_GEOMETRY_STREAMDISK_LAYOUT*)(binaryData.data() + offset);
            offset += sizeof(LWO_GEOMETRY_STREAMDISK_LAYOUT);
        }

        // Temporary for rev-eng
        RawLWOHeader.insert(RawLWOHeader.begin(), binaryData.begin(), binaryData.end());
        return;
    }

    void LWO::Serialize(LWO_HEADER lwoHeader, std::vector<uint8_t> lwoGeo)
    {
        Header = lwoHeader;
        Geo.resize(Header.Metadata.NumMeshes);

        uint64_t offset = 0;
        uint64_t offsetNormals = 0;
        uint64_t offsetUVs = 0;
        uint64_t offsetFaces = 0;

        // Get offsets from LWO streamdb data
        if (Header.MeshInfo[0].BMLHeaders[0].LWOVersion == 60)
        {
            offsetNormals = Header.LWOStreamDBData[0].LOD_NormalStartOffset;
            offsetUVs = Header.LWOStreamDBData[0].LOD_UVStartOffset;
            offsetFaces = Header.LWOStreamDBData[0].LOD_FacesStartOffset;
        }
        else
        {
            offsetNormals = Header.LWOStreamDBDataVariant[0].LOD_NormalStartOffset;
            offsetUVs = Header.LWOStreamDBDataVariant[0].LOD_UVStartOffset;
            offsetFaces = Header.LWOStreamDBDataVariant[0].LOD_FacesStartOffset;
        }

        // Allocate
        for (int i = 0; i < Geo.size(); i++)
        {
            Geo[i].Vertices.resize(Header.MeshInfo[i].BMLHeaders[0].NumVertices);
            Geo[i].Normals.resize(Header.MeshInfo[i].BMLHeaders[0].NumVertices);
            Geo[i].UVs.resize(Header.MeshInfo[i].BMLHeaders[0].NumVertices);
            Geo[i].Faces.resize(Header.MeshInfo[i].BMLHeaders[0].NumFacesX3 / 3);
        }

        // Read Vertices
        for (int i = 0; i < Geo.size(); i++)
        {
            for (int j = 0; j < Geo[i].Vertices.size(); j++)
            {
                LWO_VERTEX_PACKED packedVertex = *(LWO_VERTEX_PACKED*)(lwoGeo.data() + offset);
                Geo[i].Vertices[j] = Geo[i].UnpackVertex(packedVertex, Header.MeshInfo[i].BMLHeaders[0].VertexOffsetX, Header.MeshInfo[i].BMLHeaders[0].VertexOffsetY, Header.MeshInfo[i].BMLHeaders[0].VertexOffsetZ, Header.MeshInfo[i].BMLHeaders[0].VertexScale);
                offset += sizeof(LWO_VERTEX_PACKED);
            }
        }
     
        // Read Normals
        offset = offsetNormals;
        for (int i = 0; i < Geo.size(); i++)
        {
            for (int j = 0; j < Geo[i].Normals.size(); j++)
            {
                LWO_NORMAL_PACKED packedNormal = *(LWO_NORMAL_PACKED*)(lwoGeo.data() + offset);
                Geo[i].Normals[j] = Geo[i].UnpackNormal(packedNormal);
                offset += sizeof(LWO_NORMAL_PACKED);
            }
        }

        // Read UVs
        offset = offsetUVs;
        for (int i = 0; i < Geo.size(); i++)
        {
            for (int j = 0; j < Geo[i].UVs.size(); j++)
            {
                LWO_UV_PACKED packedUV = *(LWO_UV_PACKED*)(lwoGeo.data() + offset);
                Geo[i].UVs[j] = Geo[i].UnpackUV(packedUV, Header.MeshInfo[i].BMLHeaders[0].UVMapOffsetU, Header.MeshInfo[i].BMLHeaders[0].UVMapOffsetV, Header.MeshInfo[i].BMLHeaders[0].UVScale);
                offset += sizeof(LWO_UV_PACKED);
            }
        }

        // Read Faces
        offset = offsetFaces;
        for (int i = 0; i < Geo.size(); i++)
        {
            for (int j = 0; j < Geo[i].Faces.size(); j++)
            {
                Geo[i].Faces[j] = *(LWO_FACE_GROUP*)(lwoGeo.data() + offset);
                offset += sizeof(LWO_FACE_GROUP);
            }
        }
        return;
    }

    // Subroutines of LWO_GEO::Serialize - for unpacking geometry
    LWO_VERTEX LWO_GEO::UnpackVertex(LWO_VERTEX_PACKED packed, float_t offsetX, float_t offsetY, float_t offsetZ, float_t vertexScale)
    {
        LWO_VERTEX vertex;
        float packedX = packed.X;
        float packedY = packed.Y;
        float packedZ = packed.Z;

        vertex.X = ((packedX / 65535) * vertexScale) + offsetX;
        vertex.Z = -(((packedY / 65535) * vertexScale) + offsetY);
        vertex.Y = ((packedZ / 65535) * vertexScale) + offsetZ;
        return vertex;
    }

    LWO_NORMAL LWO_GEO::UnpackNormal(LWO_NORMAL_PACKED packed)
    {
        LWO_NORMAL normal;
        float packedXn = packed.Xn;
        float packedYn = packed.Yn;
        float packedZn = packed.Zn;

        normal.Xn = (packedXn / 256) * 2 - 1;
        normal.Yn = -((packedYn / 256) * 2 - 1);
        normal.Zn = (packedZn / 256) * 2 - 1;
        return normal;
    }

    LWO_UV LWO_GEO::UnpackUV(LWO_UV_PACKED packed, float_t offsetU, float_t offsetV, float_t uvScale)
    {
        LWO_UV uv;
        float packedU = packed.U;
        float packedV = packed.V;

        uv.U = ((packedU / 65535) * uvScale) + offsetU;
        uv.V = abs(((abs(packedV / 65535)) * uvScale) - (1 - offsetV));
        return uv;
    }
}