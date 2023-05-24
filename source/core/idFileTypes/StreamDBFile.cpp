#include <optional>
#include "StreamDBFile.h"

#include "../Utilities.h"

namespace HAYDEN {

    // Reads a binary StreamDB file from local filesystem
    StreamDBFile::StreamDBFile(const fs::path &filePath) {
        m_filePath = filePath;
        std::ifstream f(filePath, std::ios_base::in | std::ios_base::binary);
        if (!f) {
            fprintf(stderr, "ERROR : StreamDBFile : Failed to open %ls for reading.\n", filePath.c_str());
            return;
        }

        // Read .streamdb file header
        f.seekg(0, std::ios_base::beg);
        readT(f, m_streamDBHeader);
        m_streamDBEntries.resize(m_streamDBHeader.m_numEntries);

        // Read .streamdb file entries
        for (int i = 0; i < m_streamDBHeader.m_numEntries; i++)
            readT(f, m_streamDBEntries[i]);

        f.close();
        m_loaded = true;
    }

    // Attempts to locate a StreamDBEntry based on a known streamedFileID + streamedDataLength.
    const StreamDBEntry *
    StreamDBFile::locateStreamDBEntry(uint64_t streamedFileID, uint64_t streamedDataLength) const {
        for (uint64_t i = 0; i < m_streamDBHeader.m_numEntries; i++) {
            // Match
            if ((streamedFileID == m_streamDBEntries[i].m_fileID) &&
                (streamedDataLength == m_streamDBEntries[i].m_compressedSize))
                return &m_streamDBEntries[i];

            // FileID matches, but compressed size doesn't. 
            if ((streamedFileID == m_streamDBEntries[i].m_fileID) &&
                (streamedDataLength < m_streamDBEntries[i].m_compressedSize)) {
                // Sometimes it will match the next entry in sequence, so check this.
                if (streamedDataLength == m_streamDBEntries[i + 1].m_compressedSize)
                    return &m_streamDBEntries[i + 1];
            }
        }

        // No match found, return empty StreamDBEntry
        return nullptr;
    }

    // Extracts an embedded file in StreamDB, returning it as a byte vector
    std::vector<uint8_t>
    StreamDBFile::GetEmbeddedFile(const std::string &streamDBFileName, const StreamDBEntry *streamDBEntry) const {
        // Multiply streamDBEntry.m_offset16 by 16 to get the real file offset
        uint64_t fileOffset= streamDBEntry->m_offset16 * 16;

        // Read from stream
        std::ifstream f;
        std::vector<uint8_t> fileData(streamDBEntry->m_compressedSize);
        f.open(streamDBFileName, std::ios::in | std::ios::binary);
        f.seekg(fileOffset, std::ios_base::beg);
        f.read(reinterpret_cast<char *>(fileData.data()), streamDBEntry->m_compressedSize);

        // Validate number of bytes read, 0 if failed
        fileData.resize(f.gcount());
        f.close();

        return std::move(fileData);
    }


    std::optional<std::vector<uint8_t>> StreamDBFile::getData(uint64_t resourceHash, uint64_t streamSize) const {
        for (int i = 0; i < m_streamDBEntries.size(); i++) {
            const StreamDBEntry &item = m_streamDBEntries[i];
            if (resourceHash == item.m_fileID && streamSize == item.m_compressedSize) {
                return std::move(getEntryData(item));
            }
            // FileID matches, but compressed size doesn't.
            if (resourceHash == item.m_fileID && streamSize < item.m_compressedSize) {
//                 Sometimes it will match the next entry in sequence, so check this.
                if (streamSize == m_streamDBEntries[i + 1].m_compressedSize)
                    return std::move(getEntryData(m_streamDBEntries[i + 1]));
            }
        }
        return std::nullopt;
    }

    std::vector<uint8_t> StreamDBFile::getEntryData(const StreamDBEntry &entry) const {
        uint64_t fileOffset = entry.m_offset16;
        fileOffset = fileOffset * 16;

        // Read from stream
        std::ifstream f;
        std::vector<uint8_t> fileData(entry.m_compressedSize);
        f.open(m_filePath, std::ios::in | std::ios::binary);
        f.seekg(fileOffset, std::ios_base::beg);
        f.read(reinterpret_cast<char *>(fileData.data()), entry.m_compressedSize);

        // Validate number of bytes read, 0 if failed
        fileData.resize(f.gcount());
        f.close();

        return std::move(fileData);
    }
}