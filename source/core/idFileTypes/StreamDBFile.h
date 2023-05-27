#pragma once

#include <fstream>
#include <vector>
#include <filesystem>

#include "../Utilities.h"


namespace fs = std::filesystem;
namespace HAYDEN {
    HAYDEN_PACK(

            struct StreamDBHeader // 0x20 bytes
            {
                /* 0x00 */ uint64_t m_magic;            // Always 0x50A5C2292EF3C761
                /* 0x08 */ uint32_t m_dataStartOffset;    // End of StreamDB Index tables, start of embedded files
                /* 0x0C */ uint32_t m_pad0;            // null padding
                /* 0x10 */ uint32_t m_pad1;            // null padding
                /* 0x14 */ uint32_t m_pad2;            // null padding
                /* 0x18 */ uint32_t m_numEntries;        // Does not include prefetch entries
                /* 0x1C */ uint32_t m_flags;            // Always 3
            };

            struct StreamDBEntry // 0x10 bytes
            {
                /* 0x00 */ uint64_t m_fileID;             // Shuffled version of StreamResourceHash from .resources file
                /* 0x08 */ uint32_t m_offset16;           // Multiply by 16 for the real offset
                /* 0x0C */ uint32_t m_compressedSize;
            };
    )

    class StreamDBFile {
    public:

        explicit StreamDBFile(const fs::path &path);


        [[nodiscard]] bool contain(uint64_t resourceHash) const {
            return std::any_of(m_streamDBEntries.begin(), m_streamDBEntries.end(),
                               [resourceHash](HAYDEN::StreamDBEntry item) {
                                   return item.m_fileID == resourceHash;
                               }
            );
        }

        [[nodiscard]] const fs::path &filePath() const { return m_filePath; };

        [[nodiscard]] bool loaded() const { return m_loaded; };

        [[nodiscard]] std::optional<std::vector<uint8_t>> getData(uint64_t resourceHash, uint64_t streamSize) const;

    private:

        [[nodiscard]] std::vector<uint8_t> getEntryData(const StreamDBEntry &entry) const;

        bool m_loaded = false;
        fs::path m_filePath;
        StreamDBHeader m_streamDBHeader{};
        std::vector<StreamDBEntry> m_streamDBEntries{};
    };
}