#pragma once

#include <vector>
#include <cmath>
#include <cstdint>

#include "../Common.h"

#pragma pack(push)	// Not portable, sorry.
#pragma pack(1)		// Works on my machine (TM).

namespace HAYDEN
{
    struct PackedVertex
    {
        uint16_t X = 0;
        uint16_t Y = 0;
        uint16_t Z = 0;
        uint16_t NullPad = 0;
    };

    struct Vertex
    {
        float_t X = 0;
        float_t Y = 0;
        float_t Z = 0;
    };

    struct Normal
    {
        float_t Xn = 0;
        float_t Yn = 0;
        float_t Zn = 0;
    };

    struct PackedNormal
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

    struct UV
    {
        float_t U = 0;
        float_t V = 0;
    };

    struct PackedUV
    {
        uint16_t U = 0;
        uint16_t V = 0;
    };

    struct Face
    {
        uint16_t F1 = 0;
        uint16_t F2 = 0;
        uint16_t F3 = 0;
    };

    class Mesh
    {
        public:
            std::vector<Vertex> Vertices;
            std::vector<Normal> Normals;
            std::vector<UV> UVs;
            std::vector<Face> Faces;
            Vertex UnpackVertex(PackedVertex packedVertex, float_t offsetX, float_t offsetY, float_t offsetZ, float_t vertexScale);
            Normal UnpackNormal(PackedNormal packedNormal);
            UV UnpackUV(PackedUV packedUV, float_t offsetU, float_t offsetV, float_t uvScale);
    };

    class StreamedGeometry
    {
        std::vector<Mesh> Meshes;
        void Serialize(const std::vector<uint8_t> binaryData, const int numVertices, const int numFaces, const GEO_FLAGS geoFlags, const GEO_METADATA geoMeta);
    };
}

#pragma pack(pop)