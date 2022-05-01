#include "ResourceFile.h"

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
            _FileEntries.resize(_Header.NumFileEntries);

            // Read .resources file entries
            for (uint64_t i = 0; i < _FileEntries.size(); i++)
                fread(&_FileEntries[i], sizeof(ResourceFileEntry), 1, f);

            // Read total # of strings in resource file
            fread(&_NumStrings, sizeof(uint64_t), 1, f);

            // Allocate vectors
            _PathStringIndexes.resize(_Header.NumPathStringIndexes);
            _StringEntries.resize(_NumStrings);
            _StringOffsets.resize(_NumStrings + 1);
            _StringOffsets[_NumStrings] = _Header.AddrDependencyEntries; // to find last entry string length

            // Read string offsets into vector
            for (uint64_t i = 0; i < _NumStrings; i++)
                fread(&_StringOffsets[i], 8, 1, f);

            // Read strings into vector
            for (uint64_t i = 0; i < _NumStrings; i++)
            {
                int stringLength = _StringOffsets[i + 1] - _StringOffsets[i];
                char* stringBuffer = new char[stringLength];
                fread(stringBuffer, stringLength, 1, f);
                _StringEntries[i] = stringBuffer;
            }

            // Skip ahead to string indexes
            uint64_t addrPathStringIndexes = _Header.AddrDependencyIndexes + (_Header.NumDependencyIndexes * sizeof(int));
            fseek(f, addrPathStringIndexes, SEEK_SET);

            // Read string indexes into vector
            for (uint64_t i = 0; i < _Header.NumPathStringIndexes; i++)
                fread(&_PathStringIndexes[i], sizeof(uint64_t), 1, f);
        }
        fclose(f);
    }
}