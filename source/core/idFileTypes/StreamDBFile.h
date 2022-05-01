#pragma once

#include <fstream>
#include <vector>
#include <filesystem>

#pragma pack(push)  // Not portable, sorry.
#pragma pack(1)     // Works on my machine (TM).

namespace fs = std::filesystem;

namespace HAYDEN
{
    struct StreamDBHeader // 0x20 bytes
    {
        /* 0x00 */ uint64_t Magic = 0;		    // Always 0x50A5C2292EF3C761
        /* 0x08 */ uint32_t DataStartOffset = 0;    // End of StreamDB Index tables, start of embedded files
        /* 0x0C */ uint32_t Pad0 = 0;		    // null padding
        /* 0x10 */ uint32_t Pad1 = 0;		    // null padding
        /* 0x14 */ uint32_t Pad2 = 0;		    // null padding
        /* 0x18 */ uint32_t NumEntries = 0;	    // Does not include prefetch entries
        /* 0x1C */ uint32_t Flags = 0;		    // Always 3
    };

    struct StreamDBEntry // 0x10 bytes
    {
        /* 0x00 */ uint64_t FileID = 0;             // Shuffled version of StreamResourceHash from .resources file
        /* 0x08 */ uint32_t Offset16 = 0;           // Multiply by 16 for the real offset
        /* 0x0C */ uint32_t CompressedSize = 0;     
    };

    class StreamDBFile
    {
        public:

            std::string FilePath;
            StreamDBEntry LocateStreamDBEntry(const uint64_t streamedFileID, const uint64_t streamedDataLength) const;
            std::vector<uint8_t> GetEmbeddedFile(const std::string streamDBFileName, const StreamDBEntry streamDBEntry) const;
            StreamDBFile(const fs::path& path);

        private:

            // Binary data within the .streamdb file
            StreamDBHeader _StreamDBHeader;
            std::vector<StreamDBEntry> _StreamDBEntries;
    };
}

#pragma pack(pop)