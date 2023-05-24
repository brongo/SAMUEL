#pragma once

#include <vector>
#include <string>
#include <cmath>

#include "ResourceFileReader.h"
#include "source/core/cast/cast.h"

namespace HAYDEN {
    struct GeoMetadata {
        Cast::Vector3 NegBounds{};
        Cast::Vector3 PosBounds{};

        Cast::Vector3 VertexOffset{};
        float_t VertexScale = 0;

        Cast::Vector2 UVMapOffset{};
        float_t UVScale = 0;
    };

    struct GeoFlags {
        uint16_t Flags1 = 0;               // indicates model type. 60 is standard. 124 = UV Lightmap
        uint16_t Flags2 = 0;               // always 2 (?)
    };
}
