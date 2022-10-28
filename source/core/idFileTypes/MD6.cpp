#include "MD6.h"

namespace HAYDEN
{
    void MD6_HEADER::ReadBinaryHeader(const std::vector<uint8_t> binaryData)
    {
        int offset = 0;

        // Temporary for rev-eng
        RawMD6Header.insert(RawMD6Header.begin(), binaryData.begin(), binaryData.end());

        // Read header
        MD6SKL_Filename_Length = *(uint32_t*)(binaryData.data() + offset);
        offset += sizeof(uint32_t);

        MD6SKL_Filename.resize(MD6SKL_Filename_Length);
        MD6SKL_Filename = std::string(binaryData.data() + offset, binaryData.data() + offset + MD6SKL_Filename_Length);
        offset += MD6SKL_Filename_Length;

        MD6UnkFloats = *(MD6_UNK_FLOATS*)(binaryData.data() + offset);
        offset += sizeof(MD6_UNK_FLOATS);

        // Read bone info
        NumBones = *(uint16_t*)(binaryData.data() + offset);
        BoneNumbers.resize(NumBones);
        offset += sizeof(uint16_t);

        for (int i = 0; i < NumBones; i++)
        {
            BoneNumbers[i] = *(uint8_t*)(binaryData.data() + offset);
            offset += sizeof(uint8_t);
        }

        BoneInfo = *(MD6_BONE_INFO*)(binaryData.data() + offset);
        offset += sizeof(MD6_BONE_INFO);

        // Read mesh info
        NumMeshes = *(uint32_t*)(binaryData.data() + offset);
        MeshInfo.resize(NumMeshes);
        offset += sizeof(uint32_t);

        for (int i = 0; i < NumMeshes; i++)
        {
            MeshInfo[i].MeshNameStrlen = *(uint32_t*)(binaryData.data() + offset);
            offset += sizeof(uint32_t);

            MeshInfo[i].MeshName.resize(MeshInfo[i].MeshNameStrlen);
            MeshInfo[i].MeshName = std::string(binaryData.data() + offset, binaryData.data() + offset + MeshInfo[i].MeshNameStrlen);
            offset += MeshInfo[i].MeshNameStrlen;

            // Read Material2 .decl name
            MeshInfo[i].MaterialDeclStrlen = *(uint32_t*)(binaryData.data() + offset);
            offset += sizeof(uint32_t);

            MeshInfo[i].MaterialDeclName.resize(MeshInfo[i].MaterialDeclStrlen);
            MeshInfo[i].MaterialDeclName = std::string(binaryData.data() + offset, binaryData.data() + offset + MeshInfo[i].MaterialDeclStrlen);
            offset += MeshInfo[i].MaterialDeclStrlen;

            // Read MD6Mesh unknown data
            MeshInfo[i].MeshUnknowns = *(MD6_MESH_UNKNOWNS*)(binaryData.data() + offset);
            offset += sizeof(MD6_MESH_UNKNOWNS);

            // Read LODInfo - very similar to LWO BMLr stuff
            MeshInfo[i].LODInfo.resize(3);

            if (BoneInfo.UnkFloat7 == 0 && BoneInfo.UnkFloat8 == 0)
            {
                // Short meshinfo variant
                for (int j = 0; j < 1; j++)
                {
                    MeshInfo[i].LODInfo[j] = *(MD6_LOD_INFO*)(binaryData.data() + offset);
                    offset += sizeof(MD6_LOD_INFO);

                    MeshInfo[i].UnkTuple[0] = *(uint32_t*)(binaryData.data() + offset);
                    offset += sizeof(uint32_t);

                    MeshInfo[i].UnkTuple[1] = *(uint32_t*)(binaryData.data() + offset);
                    offset += sizeof(uint32_t);
                }
            }
            else
            {
                // Standard meshinfo 
                for (int j = 0; j < 3; j++)
                {
                    MeshInfo[i].LODInfo[j] = *(MD6_LOD_INFO*)(binaryData.data() + offset);
                    offset += sizeof(MD6_LOD_INFO);
                }
            }

            // Read footer unknowns
            MeshInfo[i].MeshFooter = *(MD6_MESH_FOOTER*)(binaryData.data() + offset);
            offset += sizeof(MD6_MESH_FOOTER);
        }

        // StreamDB structure depends on what our MD6 version is.
        const uint64_t streamDBStructCount = 5;
        StreamDBHeaders.resize(streamDBStructCount);
        StreamDiskLayout.resize(streamDBStructCount);
        uint64_t streamDBStructSize = sizeof(MD6_STREAMDB_HEADER) + sizeof(MD6_GEOMETRY_STREAMDISK_LAYOUT);

        // Assume we are dealing with version 60 for now - no variants
        streamDBStructSize += sizeof(MD6_STREAMDB_DATA);
        StreamDBData.resize(streamDBStructCount);

        // Move offset to the beginning of streamDB structures
        // Calculated from end of file.
        offset = binaryData.size() - (streamDBStructSize * streamDBStructCount);
        
        // Move back another 4 bytes to get NumStreams
        offset = offset - 4;

        // Read NumStreams
        NumStreams = *(uint32_t*)(binaryData.data() + offset);
        offset += 4;

        if (NumStreams != 5)
            return; // abort
       
        // Read StreamDB info
        for (int i = 0; i < streamDBStructCount; i++)
        {
            StreamDBHeaders[i] = *(MD6_STREAMDB_HEADER*)(binaryData.data() + offset);
            offset += sizeof(MD6_STREAMDB_HEADER);

            StreamDBData[i] = *(MD6_STREAMDB_DATA*)(binaryData.data() + offset);
            offset += sizeof(MD6_STREAMDB_DATA);
        }

        // Read GeoStreamDiskLayouts
        for (int i = 0; i < streamDBStructCount; i++)
        {
            StreamDiskLayout[i] = *(MD6_GEOMETRY_STREAMDISK_LAYOUT*)(binaryData.data() + offset);
            offset += sizeof(MD6_GEOMETRY_STREAMDISK_LAYOUT);
        }

        return;
    }

    void MD6::Serialize(MD6_HEADER md6Header, std::vector<uint8_t> md6Geo)
    {
        Header = md6Header;
        MeshGeometry.resize(Header.NumMeshes);

        uint64_t offset = 0;
        uint64_t offsetNormals = 0;
        uint64_t offsetUVs = 0;
        uint64_t offsetFaces = 0;

        // Get offsets from MD6 streamdb data
        offsetNormals = Header.StreamDBData[0].NormalStartOffset;
        offsetUVs = Header.StreamDBData[0].UVStartOffset;
        offsetFaces = Header.StreamDBData[0].FacesStartOffset;

        // Allocate
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            MeshGeometry[i].Vertices.resize(Header.MeshInfo[i].LODInfo[0].NumVertices);
            MeshGeometry[i].Normals.resize(Header.MeshInfo[i].LODInfo[0].NumVertices);
            MeshGeometry[i].UVs.resize(Header.MeshInfo[i].LODInfo[0].NumVertices);
            MeshGeometry[i].Faces.resize(Header.MeshInfo[i].LODInfo[0].NumFaces);
        }

        // Read Vertices
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].Vertices.size(); j++)
            {
                PackedVertex packedVertex = *(PackedVertex*)(md6Geo.data() + offset);
                MeshGeometry[i].Vertices[j] = MeshGeometry[i].UnpackVertex(packedVertex, Header.MeshInfo[i].LODInfo[0].Meta.VertexOffsetX, Header.MeshInfo[i].LODInfo[0].Meta.VertexOffsetY, Header.MeshInfo[i].LODInfo[0].Meta.VertexOffsetZ, Header.MeshInfo[i].LODInfo[0].Meta.VertexScale);
                offset += sizeof(PackedVertex);
            }
        }

        // Read Normals
        offset = offsetNormals;
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].Normals.size(); j++)
            {
                PackedNormal packedNormal = *(PackedNormal*)(md6Geo.data() + offset);
                MeshGeometry[i].Normals[j] = MeshGeometry[i].UnpackNormal(packedNormal);
                offset += sizeof(PackedNormal);
            }
        }

        // Read UVs
        offset = offsetUVs;
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].UVs.size(); j++)
            {
                PackedUV packedUV = *(PackedUV*)(md6Geo.data() + offset);
                MeshGeometry[i].UVs[j] = MeshGeometry[i].UnpackUV(packedUV, Header.MeshInfo[i].LODInfo[0].Meta.UVMapOffsetU, Header.MeshInfo[i].LODInfo[0].Meta.UVMapOffsetV, Header.MeshInfo[i].LODInfo[0].Meta.UVScale);
                offset += sizeof(PackedUV);
            }
        }

        // Read Faces
        offset = offsetFaces;
        for (int i = 0; i < MeshGeometry.size(); i++)
        {
            for (int j = 0; j < MeshGeometry[i].Faces.size(); j++)
            {
                MeshGeometry[i].Faces[j] = *(Face*)(md6Geo.data() + offset);
                offset += sizeof(Face);
            }
        }
        return;
    }
}