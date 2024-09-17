#include "ResourceFile.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    // Reads binary .resources file from local filesystem
    ResourceFile::ResourceFile(const fs::path& filePath)
    {
        FilePath = filePath.string();
        FILE* f = fopen(FilePath.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "ERROR : ResourceFile : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        if (f != NULL)
        {
            // Read .resources file header
            fseek(f, 0, SEEK_SET);
            fread(&_Header, sizeof(ResourceFileHeader), 1, f);
            _FileEntries.reserve(_Header.NumFileEntries);

            // Read .resources file entries
            ResourceFileEntry fileEntry;
            for (uint64_t i = 0; i < _FileEntries.size(); i++) {
                fread(&fileEntry, sizeof(ResourceFileEntry), 1, f);
                _FileEntries.push_back(fileEntry);
            }

            // Read total # of strings in resource file
            fread(&_NumStrings, sizeof(uint64_t), 1, f);

            // Allocate vectors
            _PathStringIndexes.reserve(_Header.NumPathStringIndexes);
            _StringEntries.reserve(_NumStrings);
            _StringOffsets.reserve(_NumStrings + 1);

            // Read string offsets into vector
            uint64_t offset;
            for (uint64_t i = 0; i < _NumStrings; i++) {
                fread(&offset, 8, 1, f);
                _StringOffsets.push_back(offset);
            }
            _StringOffsets.push_back(_Header.AddrDependencyEntries); // to find last entry string length

            // Read strings into vector
            for (uint64_t i = 0; i < _NumStrings; i++)
            {
                int stringLength = _StringOffsets[i + 1] - _StringOffsets[i];
                std::unique_ptr<char> stringBuffer(new char[stringLength]);
                fread(stringBuffer.get(), stringLength, 1, f);
                _StringEntries.emplace_back(stringBuffer.get());
            }

            // Skip ahead to string indexes
            uint64_t addrPathStringIndexes = _Header.AddrDependencyIndexes + (_Header.NumDependencyIndexes * sizeof(int));
            fseek(f, addrPathStringIndexes, SEEK_SET);

            // Read string indexes into vector
            uint64_t pathStringIndex;
            for (uint64_t i = 0; i < _Header.NumPathStringIndexes; i++) {
                fread(&pathStringIndex, sizeof(uint64_t), 1, f);
                _PathStringIndexes.push_back(pathStringIndex);
            }
        }
        fclose(f);
    }
}