#include "ResourceFile.h"

namespace HAYDEN
{
    EmbeddedTGAHeader ResourceFile::ReadTGAHeader(const std::vector<byte> tgaDecompressedHeader) const
    {
        EmbeddedTGAHeader tgaHeader;

        tgaHeader.textureMaterialKind = *(int*)(tgaDecompressedHeader.data() + 8);
        tgaHeader.imageType = *(int*)(tgaDecompressedHeader.data() + 41);
        tgaHeader.isStreamed = *(byte*)(tgaDecompressedHeader.data() + 55);
        tgaHeader.streamDBMipCount = *(int*)(tgaDecompressedHeader.data() + 59);
        tgaHeader.pixelWidth = *(int*)(tgaDecompressedHeader.data() + 71);
        tgaHeader.pixelHeight = *(int*)(tgaDecompressedHeader.data() + 75);
        tgaHeader.decompressedSize = *(int*)(tgaDecompressedHeader.data() + 83);
        tgaHeader.isCompressed = *(int*)(tgaDecompressedHeader.data() + 87);
        tgaHeader.compressedSize = *(int*)(tgaDecompressedHeader.data() + 91);
       
        // special handling for small images: these aren't in .streamdb, even if isStreamed = 1.
        if (tgaHeader.pixelHeight <= 32 && tgaHeader.pixelWidth <= 32)
            tgaHeader.isStreamed = 0;
        
        // special handling for fonts: overrides streamDBMipCount so we can calculate streamDBIndex correctly.
        if (tgaHeader.textureMaterialKind == 19)
            tgaHeader.streamDBMipCount = 11;

        // special handling for non-streaming images: this tga header contains the whole file
        if (tgaHeader.isStreamed == 0 && tgaHeader.isCompressed == 0) 
        {
            // calculate header size
            size_t mipCount = *(int*)(tgaDecompressedHeader.data() + 24);
            size_t dataStart = 63 + (36 * mipCount);
            size_t dataEnd = tgaHeader.decompressedSize + dataStart;

            // read the raw image data into byte vector
            tgaHeader.unstreamedFileData.insert(tgaHeader.unstreamedFileData.begin(), tgaDecompressedHeader.data() + dataStart, tgaDecompressedHeader.data() + dataEnd);
        }

        // special handling for $minmip= images: streamDBMipCount is encoded.
        if (tgaHeader.streamDBMipCount > 8)
        {           
            // divide by 16 to get largest mip used in game, then parse header for this data
            int mipCount = tgaHeader.streamDBMipCount / 16;
            int dataStart = 63 + (36 * mipCount);
            tgaHeader.pixelWidth = *(int*)(tgaDecompressedHeader.data() + dataStart + 8);
            tgaHeader.pixelHeight = *(int*)(tgaDecompressedHeader.data() + dataStart + 12);
            tgaHeader.decompressedSize = *(int*)(tgaDecompressedHeader.data() + dataStart + 20);
            tgaHeader.compressedSize = *(int*)(tgaDecompressedHeader.data() + dataStart + 28);

            // remainder minus mipCount = actual streamDBMipCount used for calculating streamDBIndex.
            tgaHeader.streamDBMipCount = tgaHeader.streamDBMipCount % 16 - mipCount;
        }

        // special handling for $minmip= images: streamDBMipCount is encoded.
        if (tgaHeader.streamDBMipCount > 8)
        {
            // divide by 16 to get largest mip used in game (original image isn't used)
            int largestMipUsed = tgaHeader.streamDBMipCount / 16;
            int dataStart = 63 + (36 * largestMipUsed);

            // replace original size & dimensions with data for this mip
            tgaHeader.pixelWidth = *(int*)(tgaDecompressedHeader.data() + dataStart + 8);
            tgaHeader.pixelHeight = *(int*)(tgaDecompressedHeader.data() + dataStart + 12);
            tgaHeader.decompressedSize = *(int*)(tgaDecompressedHeader.data() + dataStart + 20);
            tgaHeader.compressedSize = *(int*)(tgaDecompressedHeader.data() + dataStart + 28);

            // decode streamDBMipCount used for calculating streamDBIndex.
            tgaHeader.streamDBMipCount = tgaHeader.streamDBMipCount % 16 - largestMipUsed;
        }

        return tgaHeader;
    }
    EmbeddedMD6Header ResourceFile::ReadMD6Header(const std::vector<byte> md6DecompressedHeader) const
    {
        EmbeddedMD6Header md6Header;

        md6Header.cumulativeStreamDBSize = *(int*)(md6DecompressedHeader.data() + (md6DecompressedHeader.size() - 4));
        md6Header.decompressedSize = *(int*)(md6DecompressedHeader.data() + (md6DecompressedHeader.size() - 76));
        md6Header.compressedSize = *(int*)(md6DecompressedHeader.data() + (md6DecompressedHeader.size() - 72));

        return md6Header;
    }
    EmbeddedLWOHeader ResourceFile::ReadLWOHeader(const std::vector<byte> lwoDecompressedHeader) const
    {
        // The LWO format is very inconsistent about where it stores the streamdb info.
        // For now, only reliable way is to read 4 bytes at a time until we determine the structure size.
        // Structure size is the distance between cumulativeStreamDBSize (last 4 bytes in file), and the first time that data is repeated.

        EmbeddedLWOHeader lwoHeader;
        lwoHeader.cumulativeStreamDBSize = *(int*)(lwoDecompressedHeader.data() + (lwoDecompressedHeader.size() - 4));

        int buffer = 0;
        int structSize = 0; 
        int seekDistance = 4;
        size_t entryStart = 0;

        // Find the next instance where lwoHeader.cumulativeStreamDBSize appears. This is the struct size.
        while ((buffer != lwoHeader.cumulativeStreamDBSize) && (seekDistance < lwoDecompressedHeader.size() - 4))
        {
            seekDistance += 4;
            buffer = *(int*)(lwoDecompressedHeader.data() + (lwoDecompressedHeader.size() - seekDistance));
        }
        structSize = seekDistance - 4;

        // Seek backwards by struct size until we find 0x0000. This is the start of LOD entries.
        while ((buffer != 0) && (seekDistance < lwoDecompressedHeader.size() - 4))
        {
            seekDistance += structSize;
            buffer = *(int*)(lwoDecompressedHeader.data() + (lwoDecompressedHeader.size() - seekDistance));
        }
        entryStart = seekDistance;

        // Get decompressed & compressed sizes relative to entryStart.
        lwoHeader.decompressedSize = *(int*)(lwoDecompressedHeader.data() + (lwoDecompressedHeader.size() - (entryStart + 8)));
        lwoHeader.compressedSize = *(int*)(lwoDecompressedHeader.data() + (lwoDecompressedHeader.size() - (entryStart + 4)));
        return lwoHeader;
    }
    EmbeddedCOMPFile ResourceFile::ReadCOMPFile(const std::vector<byte> compDecompressedHeader) const
    {
        EmbeddedCOMPFile compFile;

        compFile.decompressedSize = *(int*)(compDecompressedHeader.data() + 0);
        compFile.compressedSize = *(int*)(compDecompressedHeader.data() + 8);

        compFile.unstreamedFileData = compDecompressedHeader;
        compFile.unstreamedFileData.erase(compFile.unstreamedFileData.begin(), compFile.unstreamedFileData.begin() + 16);
        return compFile;
    }

    std::vector<byte> ResourceFile::GetEmbeddedFileHeader(FILE* f, const uint64 fileOffset, const uint64 compressedSize) const
    {
        std::vector<byte> compressedData(compressedSize);
        fseek(f, (long)fileOffset, SEEK_SET);
        fread(compressedData.data(), 1, compressedSize, f);
        return compressedData;
    }

    void ResourceFile::ReadFileHeader(FILE* f)
    {
        byte buff4[4];
        byte buff8[8];

        fseek(f, 32, SEEK_SET);
        fread(buff4, 4, 1, f);
        numFileEntries = *(int*)buff4;

        fread(buff4, 4, 1, f);
        numMetaEntries = *(uint32*)buff4;

        fread(buff4, 4, 1, f);
        numMetaIndexes = *(uint32*)buff4;

        fread(buff4, 4, 1, f);
        numPathStringIndexes = *(uint32*)buff4;

        fseek(f, 56, SEEK_SET);
        fread(buff4, 4, 1, f);
        sizeStrings = *(uint32*)buff4;

        fseek(f, 64, SEEK_SET);
        fread(buff8, 8, 1, f);
        addrStringOffsets = *(uint64*)buff8;

        fseek(f, 80, SEEK_SET);
        fread(buff8, 8, 1, f);
        addrEntries = *(uint64*)buff8;

        fread(buff8, 8, 1, f);
        addrMetaEntries = *(uint64*)buff8;

        fread(buff8, 8, 1, f);
        addrPaths = *(uint64*)buff8;

        return;
    }
    void ResourceFile::ReadStringOffsets(FILE* f)
    {
        // Move pointer to string offsets
        byte buff8[8];
        fseek(f, addrStringOffsets, SEEK_SET);
        fread(buff8, 8, 1, f);

        // Calculate strings start address
        numStrings = *(uint64*)buff8;
        addrStrings = addrStringOffsets + (numStrings * sizeof(uint64));

        // Read string offsets
        offsets.resize(numStrings);
        for (int i = 0; i < numStrings; i++)
        {
            fread(buff8, 8, 1, f);
            offsets[i] = *(uint64*)buff8;
        }
        return;
    }
    void ResourceFile::ReadPathIndexes(FILE* f)
    {
        // Move pointer to path indexes
        byte buff8[8];
        addrPathIndexes = addrPaths + (numMetaIndexes * sizeof(int));
        fseek(f, addrPathIndexes, SEEK_SET);

        // Read path indexes
        for (uint32 i = 0; i < numPathStringIndexes; i++)
        {
            fread(buff8, 8, 1, f);
            pathStringIndexes.push_back(*(uint64*)buff8);
        }
        return;
    }
    void ResourceFile::ReadEntryData(FILE* f)
    {
        // Read file entries
        resourceEntries.resize(numFileEntries);
        for (uint64 i = 0; i < numFileEntries; i++)
        {
            byte buff1[1];
            byte buff4[4];
            byte buff8[8];
            char strbuff[512];
            uint64 thisEntryAddr = 0;
            uint64 pathTupleIndex = 0;
            uint64 offsetNumber = 0;
            uint64 stringOffset = 0;
            uint64 nextStringOffset = 0;
            uint64 thisStringLength = 0;
            const uint64 entrySizeInBytes = 144;

            thisEntryAddr = addrEntries + (i * entrySizeInBytes);
            fseek(f, thisEntryAddr, SEEK_SET);

            // get path index (for finding string name and type)
            fseek(f, 32, SEEK_CUR);
            fread(buff8, 8, 1, f);
            pathTupleIndex = *(uint64*)buff8;

            // data for decompressing file header
            fseek(f, 16, SEEK_CUR);
            fread(buff8, 8, 1, f);
            resourceEntries[i].dataOffset = *(uint64*)buff8;
            fread(buff8, 8, 1, f);
            resourceEntries[i].dataSizeCompressed = *(uint64*)buff8;
            fread(buff8, 8, 1, f);
            resourceEntries[i].dataSizeDecompressed = *(uint64*)buff8;

            // get hash and version
            fseek(f, 16, SEEK_CUR);
            fread(buff8, 8, 1, f);
            resourceEntries[i].hash = *(uint64*)buff8;

            fread(buff4, 4, 1, f);
            resourceEntries[i].version = *(uint32*)buff4;

            // get first havok flag
            fread(buff4, 4, 1, f);
            resourceEntries[i].havokFlag1 = *(uint32*)buff4;

            // get compression mode
            fread(buff1, 1, 1, f);
            resourceEntries[i].compressionMode = *(byte*)buff1;
            
            // get havokFlag2 and 3
            fseek(f, 1, SEEK_CUR); 
            fread(buff1, 1, 1, f);
            resourceEntries[i].havokFlag2 = *(byte*)buff1;
            fread(buff1, 1, 1, f);
            resourceEntries[i].havokFlag3 = *(byte*)buff1;

            // get string file type
            offsetNumber = pathStringIndexes[pathTupleIndex];
            stringOffset = addrStrings + offsets[offsetNumber] + 8;
            nextStringOffset = addrStrings + offsets[offsetNumber + 1] + 8;
            thisStringLength = nextStringOffset - stringOffset;

            fseek(f, stringOffset, SEEK_SET);
            fread(strbuff, 1, thisStringLength, f);
            resourceEntries[i].type = std::string(&strbuff[0], &strbuff[thisStringLength - 1]);

            // get string file name
            offsetNumber = pathStringIndexes[pathTupleIndex + 1];
            stringOffset = addrStrings + offsets[offsetNumber] + 8;

            // catch for last file entry
            if (offsetNumber + 1 == numStrings)
                nextStringOffset = addrMetaEntries;
            else
                nextStringOffset = addrStrings + offsets[offsetNumber + 1] + 8;
            thisStringLength = nextStringOffset - stringOffset;

            fseek(f, stringOffset, SEEK_SET);
            fread(strbuff, 1, thisStringLength, f);
            resourceEntries[i].name = std::string(strbuff);
            resourceEntries[i].priority = loadPriority;
        }
        return;
    }

    ResourceFile::ResourceFile(const std::string filename, const int loadPriority)
    {
        printf("Reading file %s\n", filename.c_str());
        FILE* f = fopen(filename.c_str(), "rb");
        if (f == NULL) 
        {
            fprintf(stderr, "Error: failed to open %s for reading.\n", filename.c_str());
            return;
        }

        this->filename = filename;
        this->loadPriority = loadPriority;
        ReadFileHeader(f);
        ReadStringOffsets(f);
        ReadPathIndexes(f);
        ReadEntryData(f);
        fclose(f);
    }
}
