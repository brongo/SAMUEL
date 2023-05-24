#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "idFileTypes/ResourceFile.h"

#include "Oodle.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN {


    class ResourceFileReader {
    public:
        fs::path m_resourceFilePath;

        explicit ResourceFileReader(const fs::path &resourceFilePath) { m_resourceFilePath = resourceFilePath; }

        std::vector<ResourceEntry> ParseResourceFile() const;

        [[nodiscard]] uint64_t CalculateStreamDBIndex(uint64_t resourceId, int mipCount = -6) const;

        std::vector<uint8_t> GetEmbeddedFileHeader(const std::string &resourcePath, uint64_t fileOffset,
                                                   uint64_t compressedSize, uint64_t decompressedSize);
    };
}