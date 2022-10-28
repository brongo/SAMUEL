#include "StreamDBGeometry.h"

namespace HAYDEN
{
    Vertex Mesh::UnpackVertex(PackedVertex packedVertex, float_t offsetX, float_t offsetY, float_t offsetZ, float_t vertexScale)
    {
        Vertex vertex;
        float packedX = packedVertex.X;
        float packedY = packedVertex.Y;
        float packedZ = packedVertex.Z;

        vertex.X = ((packedX / 65535) * vertexScale) + offsetX;
        vertex.Z = -(((packedY / 65535) * vertexScale) + offsetY);
        vertex.Y = ((packedZ / 65535) * vertexScale) + offsetZ;
        return vertex;
    }

    Normal Mesh::UnpackNormal(PackedNormal packedNormal)
    {
        Normal normal;
        float packedXn = packedNormal.Xn;
        float packedYn = packedNormal.Yn;
        float packedZn = packedNormal.Zn;

        normal.Xn = (packedXn / 255) * 2 - 1;
        normal.Yn = -((packedYn / 255) * 2 - 1);
        normal.Zn = (packedZn / 255) * 2 - 1;
        return normal;
    }

    UV Mesh::UnpackUV(PackedUV packed, float_t offsetU, float_t offsetV, float_t uvScale)
    {
        UV uv;
        float packedU = packed.U;
        float packedV = packed.V;

        uv.U = ((packedU / 65535) * uvScale) + offsetU;
        uv.V = abs(((abs(packedV / 65535)) * uvScale) - (1 - offsetV));
        return uv;
    }
}