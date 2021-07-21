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
            uint32 fileOffset;
            uint32 compressedSize;
    };

    class StreamDBFile
    {
        private:
            FILE* f = NULL;

        public:
            uint64 indexEntryCount = 0;
            uint32 indexStartOffset = 0;
            uint32 indexEndOffset = 0;
            std::string fileName;
            std::vector<StreamDBEntry> indexEntryList;

        public:
            // Default Constructor
            StreamDBFile() {};

            // Helper functions for organization purposes - not meant to be called individually
            bool openStreamDBFile();
            void closeStreamDBFile();
            void readStreamDBHeader();
            void readStreamDBEntries();

            // Preferred Constructor, calls helper functions above
            StreamDBFile(const fs::path& path);
    };
}