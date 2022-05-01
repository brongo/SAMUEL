#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <filesystem>

#include "../Utilities.h"

#ifdef _WIN32
#include <wincodec.h>
#include "../../../vendor/DirectXTex/DirectXTex/DirectXTex.h"
#else
#include "../../../vendor/detex/detex.h"
#endif

namespace fs = std::filesystem;

namespace HAYDEN
{
    class PNGFile
    {
        public:
            std::vector<uint8_t> ConvertDDStoPNG(std::vector<uint8_t> inputDDS);
    };
}