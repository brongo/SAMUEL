#include "ResourceFileReader.h"

namespace HAYDEN {
    // Converts .resources StreamResourceHash into a .streamdb FileID
    uint64_t ResourceFileReader::CalculateStreamDBIndex(uint64_t resourceId, int mipCount) const {
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

    // Retrieve embedded file header for a specific entry in .resources file
    std::vector<uint8_t> ResourceFileReader::GetEmbeddedFileHeader(const std::string &resourcePath, uint64_t fileOffset,
                                                                   uint64_t compressedSize, uint64_t decompressedSize) {
        FILE *f = fopen(resourcePath.c_str(), "rb");
        std::vector<uint8_t> embeddedHeader(compressedSize);

        if (f != nullptr) {
            fseek(f, (long) fileOffset, SEEK_SET);
            fread(embeddedHeader.data(), 1, compressedSize, f);

            if (embeddedHeader.size() != decompressedSize)
                embeddedHeader = oodleDecompress(embeddedHeader, decompressedSize);

            fclose(f);
        } else {
            embeddedHeader.clear();
            fprintf(stderr, "Error: failed to open %s for reading.\n", resourcePath.c_str());
        }

        return embeddedHeader;
    }

    // Public function for retrieving .resources data in a user-friendly format
    std::vector<ResourceEntry> ResourceFileReader::ParseResourceFile() const {
        // read .resources file from filesystem
        ResourceFile resourceFile(m_resourceFilePath);
        std::vector<uint64_t> pathStringIndexes = resourceFile.allPathStringIndexes();
        uint32_t numFileEntries = resourceFile.fileEntryCount();

        // allocate vector to hold all entries from this .resources file
        std::vector<ResourceEntry> resourceData;
        resourceData.resize(numFileEntries);

        // Parse each resource file and convert to usable data
        for (uint32_t i = 0; i < numFileEntries; i++) {
            const ResourceFileEntry &lexedEntry = resourceFile.resourceFileEntryAt(i);
            resourceData[i].DataOffset = lexedEntry.m_dataOffset;
            resourceData[i].DataSize = lexedEntry.m_dataSize;
            resourceData[i].DataSizeUncompressed = lexedEntry.m_dataSizeUncompressed;
            resourceData[i].Version = lexedEntry.m_version;
            resourceData[i].StreamResourceHash = lexedEntry.m_streamResourceHash;
            resourceData[i].Type = resourceFile.resourceStringEntryAt(
                    pathStringIndexes[lexedEntry.m_pathTuple_Index]);
            resourceData[i].Name = resourceFile.resourceStringEntryAt(
                    pathStringIndexes[lexedEntry.m_pathTuple_Index + 1]);
        }
        return resourceData;
    };
}