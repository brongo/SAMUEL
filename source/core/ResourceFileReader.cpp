#include "ResourceFileReader.h"

namespace HAYDEN
{
    // Converts .resources StreamResourceHash into a .streamdb FileID
    uint64_t ResourceFileReader::CalculateStreamDBIndex(uint64_t resourceId, const int mipCount) const
    {
        // Swap endianness of resourceID
        endianSwap64(resourceId);

        // Get hex bytes string
        std::string hexBytes = intToHex(resourceId);

        // Reverse each byte
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64_t)1]);

        // Shift digits to the right
        hexBytes = hexBytes.substr(hexBytes.size() - 1) + hexBytes.substr(0, hexBytes.size() - 1);

        // Reverse each byte again
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64_t)1]);

        // Get second digit based on mip count
        hexBytes[1] = intToHex((char)(6 + mipCount))[1];

        // Convert hex string back to uint64
        uint64_t streamDBIndex = hexToInt64(hexBytes);

        // Swap endian again to get streamdb index
        endianSwap64(streamDBIndex);

        return streamDBIndex;
    }

    // Retrieve embedded file header for a specific entry in .resources file
    std::vector<uint8_t> ResourceFileReader::GetEmbeddedFileHeader(const std::string resourcePath, const uint64_t fileOffset, const uint64_t compressedSize, const uint64_t decompressedSize)
    {
        FILE* f = fopen(resourcePath.c_str(), "rb");
        std::vector<uint8_t> embeddedHeader(compressedSize);

        if (f != NULL)
        {
            fseek64(f, fileOffset, SEEK_SET);
            fread(embeddedHeader.data(), 1, compressedSize, f);

            if (embeddedHeader.size() != decompressedSize)
                embeddedHeader = oodleDecompress(embeddedHeader, decompressedSize);

            fclose(f);
        }
        else
        {
            embeddedHeader.clear();
            fprintf(stderr, "Error: failed to open %s for reading.\n", resourcePath.c_str());
        }
        
        return embeddedHeader;
    }

    // Public function for retrieving .resources data in a user-friendly format
    std::vector<ResourceEntry> ResourceFileReader::ParseResourceFile()
    {
        // read .resources file from filesystem
        ResourceFile resourceFile(ResourceFilePath);
        std::vector<uint64_t> pathStringIndexes = resourceFile.GetAllPathStringIndexes();
        uint32_t numFileEntries = resourceFile.GetNumFileEntries();

        // allocate vector to hold all entries from this .resources file
        std::vector<ResourceEntry> resourceData;
        resourceData.reserve(numFileEntries);

        // Parse each resource file and convert to usable data
        for (uint32_t i = 0; i < numFileEntries; i++)
        {
            ResourceFileEntry& lexedEntry = resourceFile.GetResourceFileEntry(i);
            resourceData.push_back({
                lexedEntry.DataOffset,
                lexedEntry.DataSize,
                lexedEntry.DataSizeUncompressed,
                lexedEntry.StreamResourceHash,
                lexedEntry.Version,
                0,
                resourceFile.GetResourceStringEntry(pathStringIndexes[lexedEntry.PathTuple_Index + 1]),
                resourceFile.GetResourceStringEntry(pathStringIndexes[lexedEntry.PathTuple_Index]),
                0
            });
        }
        return resourceData;
    };

    // Public function for retrieving .PK5 data in a user-friendly format
    std::vector<ResourceEntry> ResourceFileReader::ParsePK5()
    {
        // read .resources file from filesystem
        PK5 pk5(ResourceFilePath);

        // allocate vector to hold all entries from this .PK5
        std::vector<ResourceEntry> resourceData;
        resourceData.reserve(pk5.EntryCount);

        // Parse each resource file and convert to usable data
        ResourceEntry resourceEntry;
        for (uint32_t i = 0; i < pk5.EntryCount; i++)
        {
            resourceEntry.DataOffset = pk5.FileEntries[i].DataStartOffset;
            resourceEntry.DataSize = pk5.FileEntries[i].CompressedSize;
            resourceEntry.DataSizeUncompressed = pk5.FileEntries[i].UncompressedSize;
            resourceEntry.CompressionMode = pk5.FileEntries[i].CompressionMode;
            resourceEntry.Name = pk5.EntryNames[i];

            if (pk5.EntryTypes[i] == EntryType::TYPE_IDCL)
            {
                resourceEntry.Version = pk5.EntryVersions[i];
                resourceEntry.Type = pk5.EmbeddedTypes[i];
                resourceEntry.isIDCL = 1;
            }
            else if (pk5.EntryTypes[i] == EntryType::TYPE_PLAINTEXT)
            {
                resourceEntry.Version = 0;
                resourceEntry.Type = "rs_streamfile";
            }

            resourceData.push_back(resourceEntry);
        }
        return resourceData;
    }

    // Public function for retrieving .PK5 data in a user-friendly format
    std::vector<ResourceEntry> ResourceFileReader::ParseWAD7()
    {
        // read .wad7 file from filesystem
        WAD7 wad7(ResourceFilePath);

        // allocate vector to hold all entries from this .wad7
        std::vector<ResourceEntry> resourceData;
        resourceData.reserve(wad7.EntryCount);

        // Parse each resource file and convert to usable data
        ResourceEntry resourceEntry;
        for (uint32_t i = 0; i < wad7.EntryCount; i++)
        {
            resourceEntry.DataOffset = wad7.FileEntries[i].DataStartOffset;
            resourceEntry.DataSize = wad7.FileEntries[i].CompressedSize;
            resourceEntry.DataSizeUncompressed = wad7.FileEntries[i].UncompressedSize;
            resourceEntry.CompressionMode = wad7.FileEntries[i].CompressionMode;
            resourceEntry.Name = wad7.EntryNames[i];

            if (wad7.EntryTypes[i] == EntryType::TYPE_IDCL)
            {
                resourceEntry.Version = wad7.EntryVersions[i];
                resourceEntry.Type = wad7.EmbeddedTypes[i];
                resourceEntry.isIDCL = 1;
            }
            else if (wad7.EntryTypes[i] == EntryType::TYPE_PLAINTEXT)
            {


                // Uncooked files - determine type based on file extension
                if (resourceEntry.Name.rfind(".decl") != -1)
                {
                    resourceEntry.Version = 0;
                    resourceEntry.Type = "rs_streamfile";
                }               
                else
                {
                    resourceEntry.Version = 0;
                    std::string extension = fs::path(resourceEntry.Name).extension().string();

                    if (extension.length() >= 2) {
                        resourceEntry.Type = extension.substr(1);
                    }
                    else {
                        resourceEntry.Type = "file";
                    }
                }
                    
            }

            resourceData.push_back(resourceEntry);
        }
        return resourceData;
    }
}