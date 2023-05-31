#pragma once

#include <vector>
#include <cmath>
#include <cstdint>

#include "../Common.h"
#include "../Utilities.h"


namespace HAYDEN {
    HAYDEN_PACK(
            struct PackedVertex {
                uint16_t X = 0;
                uint16_t Y = 0;
                uint16_t Z = 0;
                uint16_t NullPad = 0;
            })
    HAYDEN_PACK(
            struct PackedNormal {
                uint8_t Xn = 0;
                uint8_t Yn = 0;
                uint8_t Zn = 0;
                uint8_t weights1 = 0;
                uint8_t Xt = 0;
                uint8_t Yt = 0;
                uint8_t Zt = 0;
                uint8_t weights2 = 0;
            })
    HAYDEN_PACK(
            struct PackedUV {
                uint16_t U;
                uint16_t V;
            })
    HAYDEN_PACK(
            struct Face {
                uint16_t a;
                uint16_t b;
                uint16_t c;
            })
    HAYDEN_PACK(
            struct BoneWeights {
                float a;
                float b;
                float c;
                float d;
            })
    HAYDEN_PACK(
            struct BoneIds {
                uint8_t a;
                uint8_t b;
                uint8_t c;
                uint8_t d;
            })

    class MD6Mesh;
    class LWO;

    class Mesh {
    public:
        std::vector<Cast::Vector3> m_vertices;
        std::vector<Cast::Vector3> m_normals;
        std::vector<BoneIds> m_boneIds;
        std::vector<BoneWeights> m_boneWeights;
        std::vector<Cast::Vector2> m_uv;
        std::vector<Face> m_faces;
    private:
        static Cast::Vector3 UnpackVertex(PackedVertex packedVertex, Cast::Vector3 offset, float_t vertexScale);

        static Cast::Vector3 UnpackNormal(PackedNormal packedNormal);

        static Cast::Vector2 UnpackUV(PackedUV packedUV, Cast::Vector2 offset, float_t uvScale);

        friend MD6Mesh;
        friend LWO;

    };
}