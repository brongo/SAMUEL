#pragma once

#ifdef _WIN32
#include <WinSock2.h> // Additional dependencies: ws2_32.lib - for ntohl
#elif defined __linux__
#include <arpa/inet.h>
#endif

#include <string>
#include <vector>

typedef unsigned char byte;
typedef uint32_t uint32;
typedef uint64_t uint64;

namespace HAYDEN
{
    class EmbeddedTGAHeader
    {
        public:
            int numMips = 0;
            int isCompressed = 0;
            uint32 compressedSize = 0;
            uint32 decompressedSize = 0;
    };

    class ResourceEntry
    {
        public:
            std::string name;
            std::string type;
            uint64 hash = 0;
            uint32 version = 0;
            uint32 priority = 0;
            uint32 havokFlag1 = 0;
            byte havokFlag2 = 0;
            byte havokFlag3 = 0;
            uint32 compressionMode = 0;
            uint64 dataOffset = 0;
            uint64 dataSizeCompressed = 0;
            uint64 dataSizeDecompressed = 0;
    };

    class ResourceEntryList
    {
        public:
            uint64 numResourceEntries = 0;
            std::vector<ResourceEntry> resourceList;

        public:
            void addResourceEntry(ResourceEntry entry)
            {
                resourceList.push_back(entry);
                numResourceEntries++;
                return;
            }
            void appendResourceEntries(std::vector<ResourceEntry> appendList)
            {
                resourceList.insert(std::end(resourceList), std::begin(appendList), std::end(appendList));
                numResourceEntries = resourceList.size();
            }
    };

    class ResourceFile
    {
        public:

            // External
            std::string filename;
            int loadPriority = 0;

            // Header
            uint64 addrEntries = 0;
            uint64 addrMetaEntries = 0;
            uint64 addrPaths = 0;
            uint64 addrPathIndexes = 0;
            uint64 addrStringOffsets = 0;
            uint32 numFileEntries = 0;
            uint32 numMetaEntries = 0;
            uint32 numMetaIndexes = 0;
            uint32 numPathStringIndexes = 0;
            uint32 sizeStrings = 0;

            // Body
            uint64 addrStrings = 0;
            uint64 numStrings = 0;
            std::vector<uint64> offsets;
            std::vector<uint64> pathStringIndexes;
            std::vector<ResourceEntry> resourceEntries;

        public:
            // Default Constructor
            ResourceFile() {};

            // Helper Functions - ReadEmbeddedTGAHeaders()
            byte* GetCompressedFileHeader(FILE& f, uint64 fileOffset, uint64 compressedSize);
            EmbeddedTGAHeader ReadTGAHeader(const char* tmpDecompressedHeader);

            // Helper functions for organization purposes - not meant to be called individually
            void ReadFileHeader(FILE* f);
            void ReadStringOffsets(FILE* f);
            void ReadPathIndexes(FILE* f);
            void ReadEntryData(FILE* f);

            // Preferred Constructor, calls helper functions above
            ResourceFile(std::string filename, int loadPriority);

            
    };

}