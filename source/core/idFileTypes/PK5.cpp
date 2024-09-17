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

            FileEntries.reserve(EntryCount);
            EntryNames.reserve(EntryCount);
            EntryTypes.reserve(EntryCount);
            EntryVersions.reserve(EntryCount);
            EmbeddedTypes.reserve(EntryCount);

            // Read .PK5 file entries
            for (uint64_t i = 0; i < EntryCount; i++)
            {
                uint32_t stringLength = 0;
                fread(&stringLength, sizeof(uint32_t), 1, f);

                std::unique_ptr<char> stringBuffer(new char[stringLength]);
                fread(stringBuffer.get(), stringLength, 1, f);
                EntryNames.emplace_back(stringBuffer.get(), stringLength);

                PK5_ENTRY fileEntry;
                fread(&fileEntry, sizeof(PK5_ENTRY), 1, f);
                FileEntries.push_back(fileEntry);

                endianSwap64(FileEntries[i].DataStartOffset);
                endianSwap32(FileEntries[i].UncompressedSize);
                endianSwap32(FileEntries[i].CompressedSize);
                endianSwap32(FileEntries[i].CompressionMode);
                endianSwap32(FileEntries[i].Timestamp);
            }

            // Read first 4 bytes of each entry to identify entry type (IDCL or plaintext)
            for (uint64_t i = 0; i < EntryCount; i++)
            {
                uint32_t buffer;
                fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                fread(&buffer, 4, 1, f);

                // For IDCL files, we want to identify the contents to display in GUI
                if (buffer == 1279476809)
                {
                    EntryTypes.push_back(EntryType::TYPE_IDCL);

                    // Read IDCL header
                    fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                    ResourceFileHeader idclHeader;
                    fread(&idclHeader, sizeof(ResourceFileHeader), 1, f);

                    // Get version - for this type of IDCL only the first entry matters
                    fseek(f, FileEntries[i].DataStartOffset, SEEK_SET);
                    fseek(f, idclHeader.AddrEntries, SEEK_CUR);
                    ResourceFileEntry idclEntry;
                    fread(&idclEntry, sizeof(ResourceFileEntry), 1, f);
                    EntryVersions.push_back(idclEntry.Version);

                    // Some PK5 entries are empty due to packaging errors, skip these
                    if (idclEntry.DataSize != 0)
                    {
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