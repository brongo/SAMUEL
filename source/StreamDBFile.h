#pragma once

#include <fstream>
#include <vector>
#include <filesystem>

namespace HAYDEN
{
    namespace fs = std::filesystem;

    typedef unsigned char byte;
    typedef int16_t int16;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint64_t uint64;
    typedef int64_t int64;

    class StreamDBEntry
    {
        public:
            uint64 hashIndex;
            uint64 fileOffset;
            uint32 compressedSize;
    };

    class StreamDBFile
    {
        public:
            uint64 indexEntryCount = 0;
            uint32 indexStartOffset = 0;
            uint32 indexEndOffset = 0;
            std::string fileName;
            std::vector<StreamDBEntry> indexEntryList;

        public:
            // Default Constructor
            StreamDBFile() {};

            // Utility functions
            uint64 GetFileOffsetInStreamDB(const uint64 streamDBIndex, const uint64 compressedSize) const;
            std::vector<byte> StreamDBFile::GetEmbeddedFile(std::ifstream& f, const uint64 fileOffset, const uint64 compressedSize) const;

            // Helper functions for constructor
            void readStreamDBHeader(FILE* f);
            void readStreamDBEntries(FILE* f);

            // Preferred Constructor, calls helper functions above
            StreamDBFile(const fs::path& path);
    };
}