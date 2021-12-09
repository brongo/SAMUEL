#include "ResourceFileReader.h"

namespace HAYDEN
{
    EmbeddedTGAHeader ResourceFileReader::ReadTGAHeader(const std::vector<uint8_t> tgaDecompressedHeader) const
    {
        EmbeddedTGAHeader tgaHeader;

        tgaHeader.textureMaterialKind = *(int*)(tgaDecompressedHeader.data() + 8);
        tgaHeader.imageType = *(int*)(tgaDecompressedHeader.data() + 41);
        tgaHeader.isStreamed = *(uint8_t*)(tgaDecompressedHeader.data() + 55);
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

            // read the raw image data into uint8_t vector
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
    EmbeddedMD6Header ResourceFileReader::ReadMD6Header(const std::vector<uint8_t> md6DecompressedHeader) const
    {
        EmbeddedMD6Header md6Header;

        md6Header.cumulativeStreamDBSize = *(int*)(md6DecompressedHeader.data() + (md6DecompressedHeader.size() - 4));
        md6Header.decompressedSize = *(int*)(md6DecompressedHeader.data() + (md6DecompressedHeader.size() - 76));
        md6Header.compressedSize = *(int*)(md6DecompressedHeader.data() + (md6DecompressedHeader.size() - 72));
        md6Header.unstreamedFileHeader = md6DecompressedHeader; // Temporary for rev-eng

        return md6Header;
    }
    EmbeddedLWOHeader ResourceFileReader::ReadLWOHeader(const std::vector<uint8_t> lwoDecompressedHeader) const
    {
        // The LWO format is very inconsistent about where it stores the streamdb info.
        // For now, only reliable way is to read 4 uint8_ts at a time until we determine the structure size.
        // Structure size is the distance between cumulativeStreamDBSize (last 4 uint8_ts in file), and the first time that data is repeated.

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

        lwoHeader.unstreamedFileHeader = lwoDecompressedHeader; // Temporary for rev-eng
        return lwoHeader;
    }
    EmbeddedCOMPFile ResourceFileReader::ReadCOMPFile(const std::vector<uint8_t> compDecompressedHeader) const
    {
        EmbeddedCOMPFile compFile;

        compFile.decompressedSize = *(int*)(compDecompressedHeader.data() + 0);
        compFile.compressedSize = *(int*)(compDecompressedHeader.data() + 8);

        compFile.unstreamedFileData = compDecompressedHeader;
        compFile.unstreamedFileData.erase(compFile.unstreamedFileData.begin(), compFile.unstreamedFileData.begin() + 16);
        return compFile;
    }

    // Public function for retrieving embedded file data for a specific file entry
    std::vector<uint8_t> ResourceFileReader::GetEmbeddedFileHeader(FILE* f, const uint64_t fileOffset, const uint64_t compressedSize) const
    {
        std::vector<uint8_t> compressedData(compressedSize);
        fseek(f, (long)fileOffset, SEEK_SET);
        fread(compressedData.data(), 1, compressedSize, f);
        return compressedData;
    }

    // Public function for retrieving .resources data in a user-friendly format
    std::vector<ResourceEntry> ResourceFileReader::ReadResourceFile()
    {
        // read .resources file from filesystem
        ResourceFile resourceFile(ResourceFilePath);
        std::vector<uint64_t> pathStringIndexes = resourceFile.GetAllPathStringIndexes();
        uint32_t numFileEntries = resourceFile.GetNumFileEntries();

        // allocate vector to hold all entries from this .resources file
        std::vector<ResourceEntry> resourceData;
        resourceData.resize(numFileEntries);

        // Parse each resource file and convert to usable data
        for (uint32_t i = 0; i < numFileEntries; i++)
        {
            ResourceFileEntry& lexedEntry = resourceFile.GetResourceFileEntry(i);
            resourceData[i].DataOffset = lexedEntry.DataOffset;
            resourceData[i].DataSize = lexedEntry.DataSize;
            resourceData[i].DataSizeUncompressed = lexedEntry.DataSizeUncompressed;
            resourceData[i].Version = lexedEntry.Version;
            resourceData[i].StreamResourceHash = lexedEntry.StreamResourceHash;
            resourceData[i].Type = resourceFile.GetResourceStringEntry(pathStringIndexes[lexedEntry.PathTuple_Index]);
            resourceData[i].Name = resourceFile.GetResourceStringEntry(pathStringIndexes[lexedEntry.PathTuple_Index + 1]);
        }
        return resourceData;
    };
}