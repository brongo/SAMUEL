#include "StreamDBFile.h"

namespace HAYDEN
{
    // Open .streamdb filestream. Return 1 on success, 0 on failure.open_s. 
    bool StreamDBFile::openStreamDBFile()
    {
        f = fopen(fileName.c_str(), "rb");
        if (f == NULL)
        {
            printf("Error: failed to open %s for reading.\n", fileName.c_str());
            return 0;
        }
        return 1;
    }
 
    // Close .streamdb filestream.
    void StreamDBFile::closeStreamDBFile()
    {
        if (f != NULL)
            fclose(f);
        f = NULL;
        return;
    }

    // Read binary filestream, populate StreamDBFile object (header).
    void StreamDBFile::readStreamDBHeader()
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

    // Read binary filestream, populate StreamDBFile object (entries).
    void StreamDBFile::readStreamDBEntries()
    {
        byte buff4[4];
        byte buff8[8];
        indexEntryList.resize(indexEntryCount);

        for (uint64 i = 0; i < indexEntryCount; i++)
        {
            fread(buff8, 1, 8, f);
            indexEntryList[i].hashIndex = *(uint64*)buff8;

            fread(buff4, 1, 4, f);
            indexEntryList[i].fileOffset = 16 * (*(uint32*)buff4);

            fread(buff4, 1, 4, f);
            indexEntryList[i].compressedSize = *(uint32*)buff4;
        }
        return;
    }

    // Constructor. Calls all functions above.
    StreamDBFile::StreamDBFile(fs::path& path)
    {
        fileName = path.string();
        if (openStreamDBFile())
        {
            readStreamDBHeader();
            readStreamDBEntries();
            closeStreamDBFile();
        }
    }
}