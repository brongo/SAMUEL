#include "StreamDBGeometry.h"

namespace HAYDEN {
    Cast::Vector3 Mesh::UnpackVertex(PackedVertex pv, Cast::Vector3 offset, float vertexScale) {
        const float scale = vertexScale / 65535;
        return {
                (float) pv.X * scale + offset.x,
                (float) pv.Z * scale + offset.z,
                -(((float) pv.Y * scale) + offset.y)
        };
    }

    Cast::Vector3 Mesh::UnpackNormal(PackedNormal pn) {
        const float scale = 2.0f / 255;
        return {
                (float) pn.Xn * scale - 1,
                -((float) pn.Yn * scale - 1),
                (float) pn.Zn * scale - 1
        };
    }

    Cast::Vector2 Mesh::UnpackUV(PackedUV pv, Cast::Vector2 offset, float uvScale) {
        const float scale = uvScale / 65535;
        return {
                (float) pv.U * scale + offset.x,
                fabs(fabs((float) pv.V * scale) - (1 - offset.y))
        };
    }
}