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
    struct MaterialInfo;
    class CAST {
    public:
//        void ConvertFromLWO(LWO &lwo, const std::vector<MaterialInfo>& vector);

        void ConvertFromMD6(const MD6Mesh &md6Mesh, const MD6Skl &md6Skl, const std::vector<MaterialInfo>& materials);

        void toFile(std::ofstream &stream);

        void addRoot(Cast::CastNode node) {
            m_roots.push_back(std::move(node));
        }

    private:
        std::vector<Cast::CastNode> m_roots;
    };


}