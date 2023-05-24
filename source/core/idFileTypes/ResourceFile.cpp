#include "ResourceFile.h"

#include <fstream>
#include <iostream>

#include "../Utilities.h"
#include "source/core/Oodle.h"

namespace HAYDEN {
    // Reads binary .resources file from local filesystem
    ResourceFile::ResourceFile(const fs::path &filePath) {
        m_loaded = false;
        m_filePath = filePath;
        std::ifstream f(filePath, std::ios_base::in | std::ios_base::binary);
        if (!f) {
            fprintf(stderr, "ERROR : ResourceFile : Failed to open %ls for reading.\n", m_filePath.c_str());
            return;
        }

        // Read .resources file header
        f.seekg(0, std::ios_base::beg);
        readT(f, m_header);
        m_file_entries.resize(fileEntryCount());

        // Read .resources file entries
        for (auto &m_file_entrie: m_file_entries)
            readT(f, m_file_entrie);

        // Read total # of strings in resource file
        uint64_t numStrings; //immediately after last ResourceFileEntry
        readT(f, numStrings);

        // Allocate vectors
        m_pathStringIndexes.resize(m_header.m_numPathStringIndexes);
        m_stringEntries.resize(numStrings);
        std::vector<uint64_t> stringOffsets;                   // immediately after _numStrings
        stringOffsets.resize(numStrings + 1);
        stringOffsets[numStrings] = m_header.m_addrDependencyEntries; // to find last entry string length

        // Read string offsets into vector
        for (uint64_t i = 0; i < numStrings; i++)
            readT(f, stringOffsets[i]);

        // Read strings into vector
        for (uint64_t i = 0; i < numStrings; i++) {
            size_t stringLength = stringOffsets[i + 1] - stringOffsets[i];
            char *stringBuffer = new char[stringLength];
            f.read(stringBuffer, (std::streamsize) stringLength);
            m_stringEntries[i] = stringBuffer;
            delete[] stringBuffer;
        }

        // Skip ahead to string indexes
        size_t addrPathStringIndexes =
                m_header.m_addrDependencyIndexes + (m_header.m_numDependencyIndexes * sizeof(int));
        f.seekg((std::streamoff) addrPathStringIndexes, std::ios_base::beg);

        // Read string indexes into vector
        for (uint64_t i = 0; i < m_header.m_numPathStringIndexes; i++)
            readT(f, m_pathStringIndexes[i]);
        m_loaded = true;
    }

    std::optional<std::vector<uint8_t>> ResourceFile::queryFileByName(const std::string &name) const {
        for (const auto &item: m_file_entries) {
            if (m_stringEntries.at(m_pathStringIndexes.at(item.m_pathTuple_Index + 1)) == name) {
                return std::move(readHeader(item));
            }
        }
        return std::nullopt;
    }

    std::optional<std::vector<uint8_t>>
    ResourceFile::queryStreamDataByName(const std::string &name, uint64_t streamSize) const {
        for (const auto &item: m_file_entries) {
            if (m_stringEntries.at(m_pathStringIndexes.at(item.m_pathTuple_Index + 1)) == name) {
                uint64_t streamDbResourceHash = calculateStreamDBIndex(item.m_streamResourceHash);
                for (const HAYDEN::StreamDBFile &streamFile: m_streamFiles) {
                    if (streamFile.contain(streamDbResourceHash)) {
                        auto value = streamFile.getData(streamDbResourceHash, streamSize);
                        if (!value.has_value())
                            continue;

                        return std::move(value.value());
                    }
                }
            }
        }
        return std::nullopt;
    }

    bool ResourceFile::mountStreamDB(const fs::path &path) {
        StreamDBFile streamDbFile(path);
        if (streamDbFile.loaded()) {
            m_streamFiles.push_back(std::move(streamDbFile));
            return true;
        }
        std::cerr << "Error: Failed to load " << path << " !" << std::endl;
        return false;
    }

    std::vector<uint8_t> ResourceFile::readHeader(const ResourceFileEntry &entry) const {
        std::ifstream f(m_filePath, std::ios_base::in | std::ios_base::binary);

        f.seekg(entry.m_dataOffset);
        std::vector<uint8_t> headerData;
        std::vector<uint8_t> compressedHeaderData(entry.m_dataSize);
        f.read(reinterpret_cast<char *>(compressedHeaderData.data()), compressedHeaderData.size());

        if (entry.m_dataSize != entry.m_dataSizeUncompressed)
            headerData = oodleDecompress(compressedHeaderData, entry.m_dataSizeUncompressed);
        else
            headerData = compressedHeaderData;
        f.close();

        return std::move(headerData);
    }

    uint64_t ResourceFile::calculateStreamDBIndex(uint64_t resourceId, int mipCount) {
        // Swap endianness of resourceID
        endianSwap(resourceId);

        // Get hex bytes string
        std::string hexBytes = intToHex(resourceId);

        // Reverse each byte
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64_t) 1]);

        // Shift digits to the right
        hexBytes = hexBytes.substr(hexBytes.size() - 1) + hexBytes.substr(0, hexBytes.size() - 1);

        // Reverse each byte again
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64_t) 1]);

        // Get second digit based on mip count
        hexBytes[1] = intToHex((char) (6 + mipCount))[1];

        // Convert hex string back to uint64
        uint64_t streamDBIndex = hexToInt64(hexBytes);

        // Swap endian again to get streamdb index
        endianSwap(streamDBIndex);

        return streamDBIndex;
    }

    std::vector<std::pair<std::string, std::string>> ResourceFile::filenameList() const {
        std::vector<std::pair<std::string, std::string>> allFiles;
        allFiles.reserve(m_file_entries.size());
        for (const auto &item: m_file_entries) {
            allFiles.emplace_back(m_stringEntries.at(m_pathStringIndexes.at(item.m_pathTuple_Index + 1)),
                                  m_stringEntries.at(m_pathStringIndexes.at(item.m_pathTuple_Index)));
        }
        return std::move(allFiles);
    }

    std::vector<HAYDEN::ResourceEntry> ResourceFile::fileList() const {
        std::vector<HAYDEN::ResourceEntry> allFiles;
        allFiles.reserve(m_file_entries.size());
        for (const auto &item: m_file_entries) {
            ResourceEntry res{
                    item.m_dataOffset,
                    item.m_dataSize,
                    item.m_dataSizeUncompressed,
                    item.m_streamResourceHash,
                    item.m_version,
                    item.m_compressionMode,
                    m_stringEntries.at(m_pathStringIndexes.at(item.m_pathTuple_Index + 1)),
                    m_stringEntries.at(m_pathStringIndexes.at(item.m_pathTuple_Index))
            };
            allFiles.emplace_back(res);
        }
        return std::move(allFiles);
    }

}