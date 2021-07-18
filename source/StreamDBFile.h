#pragma once
#include <fstream>
#include <vector>
#include <filesystem>

namespace HAYDEN
{
    namespace fs = std::filesystem;

    typedef unsigned char byte;
    typedef signed short int16;
    typedef unsigned short uint16;
    typedef unsigned int uint32;
    typedef unsigned __int64 uint64;
    typedef signed __int64 int64;

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
            StreamDBFile(fs::path& path);
    };
}