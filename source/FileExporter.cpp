#include "FileExporter.h"

namespace HAYDEN
{
    // Decompress & Parse TGA Headers embedded in .resources file. Called by ExportAll().
    std::vector<EmbeddedTGAHeader> FileExporter::ReadEmbeddedTGAHeaders(const ResourceFile& resourceFile)
    {
        std::vector<EmbeddedTGAHeader> embeddedTGAHeaders;
        FILE* f = fopen(resourceFile.filename.c_str(), "rb");
        if (f == NULL)
        {
            printf("Error: failed to open %s for reading.\n", resourceFile.filename.c_str());
            return embeddedTGAHeaders;
        }

        // Reads embedded TGA Headers for the data needed to locate file in .streamdb
        for (uint64 i = 0; i < fileExportList.size(); i++)
        {
            FileExportItem& thisFile = fileExportList[i];

            // go to file offset and read compressed file header into memory
            byte* compressedData = resourceFile.GetCompressedFileHeader(f, thisFile.resourceFileOffset, thisFile.resourceFileCompressedSize);

            // decompress with Oodle DLL
            auto decompressedData = oodleDecompress(compressedData, thisFile.resourceFileCompressedSize, thisFile.resourceFileDecompressedSize);
            delete[] compressedData;
            if (decompressedData.empty())
                continue; // error

            // get mip data from TGA
            EmbeddedTGAHeader embeddedTGAHeader = resourceFile.ReadTGAHeader(decompressedData);
            embeddedTGAHeaders.push_back(embeddedTGAHeader);
        }

        fclose(f);
        return embeddedTGAHeaders;
    }

    // Helper function called by SearchStreamDBFilesForIndex().
    int FileExporter::FindMatchingIndex(const uint64 streamDBIndex, const int streamDBNumber, const std::vector<StreamDBFile>& streamDBFiles) const
    {
        for (int i = 0; i < streamDBFiles[streamDBNumber].indexEntryCount; i++)
        {
            if (streamDBFiles[streamDBNumber].indexEntryList[i].hashIndex != streamDBIndex)
                continue;

            return i;
        }
        return -1;
    }

    // Loop through export list and match streamDBIndexes against .streamdb files in memory. Called by ExportAll().
    void FileExporter::SearchStreamDBFilesForIndex(FileExportItem& streamDBData, const std::vector<StreamDBFile>& streamDBFiles) const
    {
        for (int i = 0; i < streamDBFiles.size(); i++)
        {
            int matchIndex = FindMatchingIndex(streamDBData.streamDBIndex, i, streamDBFiles);
            if (matchIndex != -1)
            {
                streamDBData.streamDBNumber = i;
                streamDBData.streamDBFileName = streamDBFiles[i].fileName;
                streamDBData.streamDBFileOffset = streamDBFiles[i].indexEntryList[matchIndex].fileOffset * (uint64)16;
                return;
            }
        }
        return;
    }

    // Converts ResourceID to StreamDBIndex. Called by ExportAll().
    uint64 FileExporter::CalculateStreamDBIndex(const uint64 resourceId, const int mipCount) const
    {
        // Get hex bytes string
        std::string hexBytes = intToHex(resourceId);

        // Reverse each byte
        for (int i = 0; i < hexBytes.size(); i += 2) {
            std::swap(hexBytes[i], hexBytes[i + (int64)1]);
        }

        // Shift digits to the right
        hexBytes = hexBytes.substr(hexBytes.size() - 1) + hexBytes.substr(0, hexBytes.size() - 1);

        // Reverse each byte again
        for (int i = 0; i < hexBytes.size(); i += 2) {
            std::swap(hexBytes[i], hexBytes[i + (int64)1]);
        }

        // Get second digit based on mip count
        hexBytes[1] = intToHex((char)(6 + mipCount))[1];

        // Convert hex string back to uint64 and return
        return hexToInt64(hexBytes);
    }

    // Builds list of files to export from SAMUEL. Called by ExportAll().
    void FileExporter::BuildFileExportList(const ResourceFile& resourceFile)
    {
        // Iterate through currently loaded .resource entries to find supported files
        for (uint64 i = 0; i < resourceFile.numFileEntries; i++)
        {
            ResourceEntry thisEntry = resourceFile.resourceEntries[i];

            // for now we only want version == 21 (images)
            if (thisEntry.version != 21)
                continue;

            // skips entries with no data to extract
            if (thisEntry.dataSizeCompressed == 0)
                continue;

            // FIX ME: this *should* be supported.
            // skips entries where file header is not compressed
            if (thisEntry.compressionMode == 0)
                continue;

            FileExportItem exportItem;
            exportItem.resourceFileName = thisEntry.name;
            exportItem.resourceFileOffset = thisEntry.dataOffset;
            exportItem.resourceFileCompressedSize = thisEntry.dataSizeCompressed;
            exportItem.resourceFileDecompressedSize = thisEntry.dataSizeDecompressed;
            exportItem.resourceFileHash = thisEntry.hash;

            fileExportList.push_back(exportItem);
        }
        return;
    }

    void FileExporter::Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles)
    {
        BuildFileExportList(resourceFile);

        // Get embedded TGA data needed to locate files in .streamdb.
        std::vector<EmbeddedTGAHeader> embeddedTGAHeaders = ReadEmbeddedTGAHeaders(resourceFile);

        for (int i = 0; i < fileExportList.size(); i++)
        {
            FileExportItem& thisFile = fileExportList[i];

            endianSwap(thisFile.resourceFileHash);
            uint64 streamDBIndex = CalculateStreamDBIndex(thisFile.resourceFileHash, embeddedTGAHeaders[i].numMips);
            endianSwap(streamDBIndex);

            // populate streamdbData
            thisFile.streamDBIndex = streamDBIndex;
            thisFile.streamDBSizeDecompressed = embeddedTGAHeaders[i].decompressedSize;
            thisFile.streamDBSizeCompressed = embeddedTGAHeaders[i].compressedSize;

            // FIXME, should get compressionType from .resources
            if (embeddedTGAHeaders[i].isCompressed == 1)
                thisFile.streamDBCompressionType = 2;
        }

        // Loop through streamdbData and decompress files
        for (int i = 0; i < fileExportList.size(); i++)
        {
            SearchStreamDBFilesForIndex(fileExportList[i], streamDBFiles);
        }
    }
}
