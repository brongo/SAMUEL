#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "ResourceFile.h"

namespace HAYDEN
{
    // User-friendly struct for managing .resources data
    struct ResourceEntry
    {
        uint64_t DataOffset = 0;
        uint64_t DataSize = 0;
        uint64_t DataSizeUncompressed = 0;
        uint64_t StreamResourceHash = 0;
        uint32_t Version = 0;
        uint16_t CompressionMode = 0;
        std::string Name = 0;
        std::string Type = 0;
    };

    struct EmbeddedTGAHeader
    {
        uint8_t isStreamed = 0;
        int textureMaterialKind = 0;
        int streamDBMipCount = 0;
        int isCompressed = 0;
        int compressedSize = 0;
        int decompressedSize = 0;
        int pixelWidth = 0;
        int pixelHeight = 0;
        int imageType = 0;
        std::vector<uint8_t> unstreamedFileData;
    };
    struct EmbeddedMD6Header
    {
        int cumulativeStreamDBSize = 0;
        int compressedSize = 0;
        int decompressedSize = 0;
        std::vector<uint8_t> unstreamedFileHeader;
    };
    struct EmbeddedLWOHeader
    {
        int cumulativeStreamDBSize = 0;
        int compressedSize = 0;
        int decompressedSize = 0;
        std::vector<uint8_t> unstreamedFileHeader;
    };
    struct EmbeddedCOMPFile
    {
        int compressedSize = 0;
        int decompressedSize = 0;
        std::vector<uint8_t> unstreamedFileData;
    };
    struct EmbeddedDECLFile
    {
        std::vector<uint8_t> unstreamedFileData;
    };

    class ResourceFileReader
    {
        public:
            fs::path ResourceFilePath;

            // Retrieves .resources data in a user-friendly format
            std::vector<ResourceEntry> ReadResourceFile();

            // Retrieves embedded file data for a specific entry
            std::vector<uint8_t> GetEmbeddedFileHeader(FILE* f, const uint64_t fileOffset, const uint64_t compressedSize) const;
            EmbeddedTGAHeader ReadTGAHeader(const std::vector<uint8_t> tgaDecompressedHeader) const;
            EmbeddedMD6Header ReadMD6Header(const std::vector<uint8_t> md6DecompressedHeader) const;
            EmbeddedLWOHeader ReadLWOHeader(const std::vector<uint8_t> lwoDecompressedHeader) const;
            EmbeddedCOMPFile  ReadCOMPFile(const std::vector<uint8_t> compDecompressedHeader) const;

            ResourceFileReader(const fs::path resourceFilePath) { ResourceFilePath = resourceFilePath; }
    };
}