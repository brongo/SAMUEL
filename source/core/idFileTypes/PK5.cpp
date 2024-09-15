#include "PK5.h"

namespace HAYDEN
{
    // Reads binary .PK5 file from local filesystem
    PK5::PK5(const fs::path filePath)
    {
        FilePath = filePath.string();
        FILE* f = fopen(FilePath.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "ERROR : PK5 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        if (f != NULL)
        {
            // Read .PK5 file header
            fseek(f, 0, SEEK_SET);
            fread(&Header, sizeof(PK5_HEADER), 1, f);

            endianSwap64(Header.IndexStart);
            endianSwap64(Header.IndexSize);

            // Read entry count
            fseek(f, Header.IndexStart, SEEK_SET);
            fread(&EntryCount, sizeof(uint32_t), 1, f);
            endianSwap32(EntryCount);

            FileEntries.resize(EntryCount);
            EntryNames.resize(EntryCount);
            EntryTypes.resize(EntryCount);
            EntryVersions.resize(EntryCount);
            EmbeddedTypes.resize(EntryCount);

            // Read .PK5 file entries
            for (uint64_t i = 0; i < FileEntries.size(); i++)
            {
                uint32_t stringLength = 0;
                fread(&stringLength, sizeof(uint32_t), 1, f);

                std::unique_ptr<char> stringBuffer(new char[stringLength]);
                fread(stringBuffer.get(), stringLength, 1, f);
                EntryNames[i] = std::string(stringBuffer.get(), stringLength);

                fread(&FileEntries[i], sizeof(PK5_ENTRY), 1, f);

                endianSwap64(FileEntries[i].DataStartOffset);
                endianSwap32(FileEntries[i].UncompressedSize);
                endianSwap32(FileEntries[i].CompressedSize);
                endianSwap32(FileEntries[i].CompressionMode);
                endianSwap32(FileEntries[i].Timestamp);
            }

            // Read first 4 bytes of each entry to identify entry type (IDCL or plaintext)
            for (uint64_t i = 0; i < FileEntries.size(); i++)
            {
                uint32_t buffer;
                fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                fread(&buffer, 4, 1, f);

                // For IDCL files, we want to identify the contents to display in GUI
                if (buffer == 1279476809)
                {
                    EntryTypes[i] = EntryType::TYPE_IDCL;
                    
                    // Read IDCL header
                    fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                    ResourceFileHeader idclHeader;
                    fread(&idclHeader, sizeof(ResourceFileHeader), 1, f);            
                    
                    // Get version - for this type of IDCL only the first entry matters
                    fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                    fseek(f, idclHeader.AddrEntries, SEEK_CUR);
                    ResourceFileEntry idclEntry;
                    fread(&idclEntry, sizeof(ResourceFileEntry), 1, f);
                    EntryVersions[i] = idclEntry.Version;

                    // Get type string
                    uint64_t numStrings;
                    uint64_t addrStringCount = idclHeader.AddrEntries + (sizeof(ResourceFileEntry) * idclHeader.NumFileEntries);
                    fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                    fseek(f, addrStringCount, SEEK_CUR);
                    fread(&numStrings, sizeof(uint64_t), 1, f);

                    std::vector<uint64_t> stringOffsets;
                    stringOffsets.resize(numStrings + 1);
                    for (uint64_t j = 0; j < numStrings; j++)
                        fread(&stringOffsets[j], 8, 1, f);
                        
                    stringOffsets[numStrings] = idclHeader.AddrDependencyEntries;

                    // Read strings into vector
                    std::vector<std::string> stringEntries;
                    stringEntries.resize(numStrings);
                    for (uint64_t i = 0; i < numStrings; i++)
                    {
                        int stringLength = stringOffsets[i + 1] - stringOffsets[i];
                        std::unique_ptr<char> stringBuffer(new char[stringLength]);
                        fread(stringBuffer.get(), stringLength, 1, f);
                        stringEntries[i] = stringBuffer.get();
                    }

                    EmbeddedTypes[i] = stringEntries[idclEntry.PathTuple_OffsetType];
                }
                else
                {
                    EntryTypes[i] = EntryType::TYPE_PLAINTEXT;
                    EntryVersions[i] = 0;
                }
            }
        }
        fclose(f);
    }
}