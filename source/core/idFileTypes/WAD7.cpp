#include "WAD7.h"
#include <cstdint>
#include <cstdio>
#include <memory>

namespace HAYDEN
{
    // Reads binary .WAD7 file from local filesystem
    WAD7::WAD7(const fs::path filePath)
    {
        FilePath = filePath.string();
        FILE* f = fopen(FilePath.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "ERROR : WAD7 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        if (f != NULL)
        {
            // Read .WAD7 file header
            fseek(f, 0, SEEK_SET);
            fread(&Header, sizeof(WAD7_HEADER), 1, f);

            endianSwap64(Header.IndexStart);
            endianSwap64(Header.IndexSize);

            // Read entry count
            fseek64(f, Header.IndexStart, SEEK_SET);
            fread(&EntryCount, sizeof(uint32_t), 1, f);
            endianSwap32(EntryCount);

            FileEntries.reserve(EntryCount);
            EntryNames.reserve(EntryCount);
            EntryTypes.reserve(EntryCount);
            EntryVersions.reserve(EntryCount);
            EmbeddedTypes.reserve(EntryCount);

            // Read .WAD7 file entries
            WAD7_ENTRY fileEntry;
            for (uint64_t i = 0; i < EntryCount; i++)
            {
                uint32_t stringLength = 0;
                fread(&stringLength, sizeof(uint32_t), 1, f);

                std::unique_ptr<char> stringBuffer(new char[stringLength]);
                fread(stringBuffer.get(), stringLength, 1, f);
                EntryNames.emplace_back(stringBuffer.get(), stringLength);

                fread(&fileEntry, sizeof(WAD7_ENTRY), 1, f);
                FileEntries.push_back(fileEntry);

                endianSwap64(FileEntries[i].DataStartOffset);
                endianSwap32(FileEntries[i].UncompressedSize);
                endianSwap32(FileEntries[i].CompressedSize);
                endianSwap32(FileEntries[i].CompressionMode);
            }

            // Read first 4 bytes of each entry to identify entry type (IDCL or plaintext)
            for (uint64_t i = 0; i < EntryCount; i++)
            {
                fseek64(f, FileEntries[i].DataStartOffset - ftell64(f), SEEK_CUR);
                
                uint32_t buffer;
                fread(&buffer, 4, 1, f);

                // For IDCL files, we want to identify the contents to display in GUI
                if (buffer == 1279476809)
                {
                    EntryTypes.push_back(EntryType::TYPE_IDCL);

                    // Read IDCL header
                    fseek64(f, -4, SEEK_CUR);
                    ResourceFileHeader idclHeader;
                    fread(&idclHeader, sizeof(ResourceFileHeader), 1, f);            
                    
                    // Some WAD7 entries are empty, reason unknown
                    if (idclHeader.NumFileEntries != 0)
                    {
                        // Get version - for this type of IDCL only the first entry matters
                        fseek64(f, idclHeader.AddrEntries - sizeof(ResourceFileHeader), SEEK_CUR);
                        ResourceFileEntry idclEntry;
                        fread(&idclEntry, sizeof(ResourceFileEntry), 1, f);
                        EntryVersions.push_back(idclEntry.Version);

                        // Advance past any other entries
                        if (idclHeader.NumFileEntries > 1)
                            fseek64(f, sizeof(ResourceFileEntry) * (idclHeader.NumFileEntries - 1), SEEK_CUR);

                        // Get type string
                        uint64_t numStrings;
                        fread(&numStrings, sizeof(uint64_t), 1, f);

                        // Skip to offset we want
                        fseek(f, idclEntry.PathTuple_OffsetType * sizeof(uint64_t), SEEK_CUR);

                        // Get string offset
                        uint64_t offset = 0;
                        fread(&offset, sizeof(uint64_t), 1, f);

                        // Get offset after
                        uint64_t next_offset = 0;
                        fread(&next_offset, sizeof(uint64_t), 1, f);

                        // Seek to string
                        fseek(f, (numStrings - idclEntry.PathTuple_OffsetType - 1) * sizeof(uint64_t), SEEK_CUR);

                        // Read string
                        size_t stringLength = next_offset - offset;
                        std::unique_ptr<char> stringBuffer(new char[stringLength]);
                        fread(stringBuffer.get(), stringLength, 1, f);
                        EmbeddedTypes.emplace_back(stringBuffer.get());
                    }
                    else
                    {
                        // This will be hidden in GUI, nothing to export
                        EntryVersions.push_back(9999);
                        EmbeddedTypes.push_back("Empty File");
                    }

                }
                else
                {
                    EntryTypes.push_back(EntryType::TYPE_PLAINTEXT);
                    EntryVersions.push_back(0);
                    EmbeddedTypes.push_back("");
                }
            }
        }
        fclose(f);
    }
}
