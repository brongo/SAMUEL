#pragma once

#include <vector>
#include <cmath>
#include <cstdint>

#include "../Common.h"

#pragma pack(push)    // Not portable, sorry.
#pragma pack(1)        // Works on my machine (TM).

namespace HAYDEN {
    struct PackedVertex {
        uint16_t X = 0;
        uint16_t Y = 0;
        uint16_t Z = 0;
        uint16_t NullPad = 0;
    };

    struct Vertex {
        float_t X = 0;
        float_t Y = 0;
        float_t Z = 0;
    };

    struct Normal {
        float_t Xn = 0;
        float_t Yn = 0;
        float_t Zn = 0;
    };

    struct PackedNormal {
        uint8_t Xn = 0;
        uint8_t Yn = 0;
        uint8_t Zn = 0;
        uint8_t weights1 = 0;
        uint8_t Xt = 0;
        uint8_t Yt = 0;
        uint8_t Zt = 0;
        uint8_t weights2 = 0;
    };

    struct UV {
        float_t U = 0;
        float_t V = 0;
    };

    struct PackedUV {
        uint16_t U = 0;
        uint16_t V = 0;
    };

    struct Face {
        uint16_t F1 = 0;
        uint16_t F2 = 0;
        uint16_t F3 = 0;
    };

    struct BoneWeights {
        float a, b, c, d;
    };

    struct BoneIds {
        uint8_t a, b, c, d;
    };

    class Mesh {
    public:
        std::vector<Vertex> m_vertices;
        std::vector<Normal> m_normals;
        std::vector<BoneIds> m_boneIds;
        std::vector<BoneWeights> m_boneWeights;
        std::vector<UV> m_uv;
        std::vector<Face> m_faces;

        static Vertex UnpackVertex(PackedVertex packedVertex, Cast::Vector3 offset, float_t vertexScale);

        static Normal UnpackNormal(PackedNormal packedNormal);

        static UV UnpackUV(PackedUV packedUV, Cast::Vector2 offset, float_t uvScale);
    };

    class StreamedGeometry {
        std::vector<Mesh> Meshes;

        void Serialize(const std::vector<uint8_t> binaryData, const int numVertices, const int numFaces,
                       const GeoFlags geoFlags, const GeoMetadata geoMeta);
    };
}

#pragma pack(pop)