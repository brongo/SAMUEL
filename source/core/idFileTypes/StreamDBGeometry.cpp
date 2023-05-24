#include "StreamDBGeometry.h"

namespace HAYDEN {
    Vertex Mesh::UnpackVertex(PackedVertex pv, Cast::Vector3 offset, float vertexScale) {
        float scale = vertexScale / 65535;
        return Vertex{
                (float) pv.X * scale + offset.x,
                (float) pv.Z * scale + offset.z,
                (float) -((pv.Y * scale) + offset.y)
        };
    }

    Normal Mesh::UnpackNormal(PackedNormal pn) {
        float scale = 2.0f / 255;
        return Normal{
                (float) pn.Xn * scale - 1,
                -((float) pn.Yn * scale - 1),
                (float) pn.Zn * scale - 1
        };
    }

    UV Mesh::UnpackUV(PackedUV pv, Cast::Vector2 offset, float uvScale) {
        float scale = uvScale / 65535;
        return UV{
                (float) pv.U * scale + offset.x,
                fabs(fabs((float) pv.V * scale) - (1 - offset.y))
        };
    }
}