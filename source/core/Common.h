#pragma once

#include <vector>
#include <string>
#include <cmath>

#include "ResourceFileReader.h"

namespace HAYDEN
{
    struct RESOURCES_ARCHIVE
    {
        std::string ResourceName;
        fs::path ResourcePath;
        std::vector<ResourceEntry> Entries;
    };

    struct GLOBAL_RESOURCES
    {
        std::vector<RESOURCES_ARCHIVE> Files;
    };

    struct GEO_METADATA
    {
        float_t NegBoundsX = 0;
        float_t NegBoundsY = 0;
        float_t NegBoundsZ = 0;
        float_t PosBoundsX = 0;
        float_t PosBoundsY = 0;
        float_t PosBoundsZ = 0;

        float_t VertexOffsetX = 0;
        float_t VertexOffsetY = 0;
        float_t VertexOffsetZ = 0;
        float_t VertexScale = 0;

        float_t UVMapOffsetU = 0;
        float_t UVMapOffsetV = 0;
        float_t UVScale = 0;
    };

    struct GEO_FLAGS
    {
        uint16_t Flags1 = 0;               // indicates model type. 60 is standard. 124 = UV Lightmap
        uint16_t Flags2 = 0;               // always 2 (?)
    };
}