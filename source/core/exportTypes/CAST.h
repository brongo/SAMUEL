#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <map>

#include "../idFileTypes/LWO.h"
#include "../idFileTypes/MD6Mesh.h"
#include "../idFileTypes/MD6Skl.h"
#include "../cast/cast.h"

namespace fs = std::filesystem;


namespace HAYDEN {

    class CAST {
    public:
        void ConvertFromLWO(LWO &lwo);

        void ConvertFromMD6(const MD6Mesh &md6Mesh, const MD6Skl &md6Skl);

        void toFile(std::ofstream &stream);

        void addRoot(Cast::CastNode node) {
            m_roots.push_back(std::move(node));
        }

    private:
        std::vector<Cast::CastNode> m_roots;
    };


}