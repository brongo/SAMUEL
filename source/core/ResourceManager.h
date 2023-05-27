#pragma once

#include <filesystem>
#include <vector>

#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/PackageMapSpec.h"

namespace fs = std::filesystem;
namespace HAYDEN {
    class ResourceManager {
    public:
        bool mountResourceFile(const fs::path &path);

        [[nodiscard]] std::optional<std::vector<uint8_t>> queryFileByName(const std::string &name) const;

        [[nodiscard]] std::optional<std::vector<uint8_t>>
        queryStreamDataByName(const std::string &name, uint64_t streamSize, int32_t mipCount=-6) const;

        [[nodiscard]] std::vector<std::pair<std::string,std::string>> filenameList() const;
        [[nodiscard]] std::vector<ResourceEntry> fileList() const;

        void reset();

    private:
        bool loadPackageMapSpec(const fs::path &path);

        std::vector<ResourceFile> m_resourceFiles;
        PackageMapSpec m_mapSpec;
    };
}