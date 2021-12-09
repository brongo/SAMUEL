#include "StreamDBFile.h"

namespace HAYDEN
{
    uint64 StreamDBFile::GetFileOffsetInStreamDB(const uint64 streamDBIndex, const uint64 compressedSize) const
    {
        for (int i = 0; i < indexEntryCount; i++)
        {
            if (indexEntryList[i].hashIndex != streamDBIndex)
                continue;

            // make sure the compressedSize matches too
            if (indexEntryList[i].compressedSize == compressedSize)
                return indexEntryList[i].fileOffset;

            // special return value indicating index matches, but size is too small
            if (indexEntryList[i].compressedSize < compressedSize)
                return -2;
        }
        return -1;
    }
    std::vector<byte> StreamDBFile::GetEmbeddedFile(std::ifstream& f, const uint64 fileOffset, const uint64 compressedSize) const
    {
        std::vector<byte> compressedData(compressedSize);
        f.seekg(fileOffset, std::ios_base::beg);
        f.read(reinterpret_cast<char*>(compressedData.data()), compressedSize);

        // validate number of bytes read, 0 if failed
        compressedData.resize(f.gcount());
        return compressedData;
    }

    void StreamDBFile::readStreamDBHeader(FILE* f)
    {
        byte buff4[4];
        fseek(f, 8, SEEK_SET);
        fread(buff4, 1, 4, f);
        indexEndOffset = *(uint32*)buff4;

        fseek(f, 12, SEEK_CUR);
        fread(buff4, 1, 4, f);
        indexEntryCount = *(uint32*)buff4;

        fseek(f, 4, SEEK_CUR);
        indexStartOffset = ftell(f);
        return;
    }
    void StreamDBFile::readStreamDBEntries(FILE* f)
    {
        byte buff4[4];
        byte buff8[8];
        indexEntryList.resize(indexEntryCount);

        for (uint64 i = 0; i < indexEntryCount; i++)
        {
            fread(buff8, 1, 8, f);
            indexEntryList[i].hashIndex = *(uint64*)buff8;

            fread(buff4, 1, 4, f);
            uint64 fileOffset = *(uint32*)buff4;
            fileOffset = fileOffset * 16;
            indexEntryList[i].fileOffset = fileOffset;

            fread(buff4, 1, 4, f);
            indexEntryList[i].compressedSize = *(uint32*)buff4;
        }
        return;
    }

    StreamDBFile::StreamDBFile(const fs::path& path)
    {
        fileName = path.string();
        FILE* f = fopen(fileName.c_str(), "rb");
        if (f != NULL)
        {
            readStreamDBHeader(f);
            readStreamDBEntries(f);
            fclose(f);
        }
    }
}
