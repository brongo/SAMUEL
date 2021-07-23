#include "FileExporter.h"

namespace HAYDEN
{
    // FileExportList Functions
    void FileExportList::GetResourceEntries(const ResourceFile& resourceFile)
    {
        // Iterate through currently loaded .resource entries to find supported files
        for (uint64 i = 0; i < resourceFile.numFileEntries; i++)
        {
            ResourceEntry thisEntry = resourceFile.resourceEntries[i];

            // for now we only want version == 21 (images)
            if (thisEntry.version != _FileType)
                continue;

            // skips entries with no data to extract
            if (thisEntry.dataSizeCompressed == 0)
                continue;

            // skips entries with "lightprobes" path
            if (thisEntry.name.rfind("/lightprobes/") != -1)
                continue;

            FileExportItem exportItem;
            exportItem.resourceFileName = thisEntry.name;
            exportItem.resourceFileOffset = thisEntry.dataOffset;
            exportItem.resourceFileCompressedSize = thisEntry.dataSizeCompressed;
            exportItem.resourceFileDecompressedSize = thisEntry.dataSizeDecompressed;
            exportItem.resourceFileHash = thisEntry.hash;

            _ExportItems.push_back(exportItem);
        }
        return;
    }
    void FileExportList::ParseEmbeddedTGAHeaders(const ResourceFile& resourceFile)
    {
        // Open .resource file containing embedded TGA data.
        FILE* f = fopen(resourceFile.filename.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "Error: failed to open %s for reading.\n", resourceFile.filename.c_str());
            return;
        }
        
        // read compressed file headers into memory
        for (uint64 i = 0; i < _ExportItems.size(); i++)
        {
            FileExportItem& thisFile = _ExportItems[i];
            auto compressedData = resourceFile.GetEmbeddedFileHeader(f, thisFile.resourceFileOffset, thisFile.resourceFileCompressedSize);

            // check if decompression is necessary
            if (thisFile.resourceFileCompressedSize != thisFile.resourceFileDecompressedSize)
            {
                // decompress with Oodle DLL
                auto decompressedData = oodleDecompress(compressedData, thisFile.resourceFileCompressedSize, thisFile.resourceFileDecompressedSize);
                if (decompressedData.empty())
                    continue; // error

                EmbeddedTGAHeader embeddedTGAHeader = resourceFile.ReadTGAHeader(decompressedData);
                _TGAHeaderData.push_back(embeddedTGAHeader);
            }
            else
            {
                std::vector<byte> decompressedData(compressedData.begin(), compressedData.begin() + thisFile.resourceFileCompressedSize);
                EmbeddedTGAHeader embeddedTGAHeader = resourceFile.ReadTGAHeader(decompressedData);
                _TGAHeaderData.push_back(embeddedTGAHeader);
            }        
        }
        fclose(f);
        return;
    }
    uint64 FileExportList::CalculateStreamDBIndex(const uint64 resourceId, const int mipCount) const
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
    void FileExportList::GetStreamDBIndexAndSize()
    {
        for (int i = 0; i < _ExportItems.size(); i++)
        {
            FileExportItem& thisFile = _ExportItems[i];

            endianSwap(thisFile.resourceFileHash);
            uint64 streamDBIndex = CalculateStreamDBIndex(thisFile.resourceFileHash, _TGAHeaderData[i].numMips);
            endianSwap(streamDBIndex);

            // populate streamdbData
            thisFile.streamDBIndex = streamDBIndex;
            thisFile.streamDBSizeDecompressed = _TGAHeaderData[i].decompressedSize;
            thisFile.streamDBSizeCompressed = _TGAHeaderData[i].compressedSize;

            // FIXME, should get compressionType from .resources
            if (_TGAHeaderData[i].isCompressed == 1)
                thisFile.streamDBCompressionType = 2;
        }
        return;
    }

    // Constructs FileExportList
    FileExportList::FileExportList(const ResourceFile& resourceFile, int fileType)
    {
        _FileType = fileType;
        _ResourceFileName = resourceFile.filename;
        GetResourceEntries(resourceFile);
        ParseEmbeddedTGAHeaders(resourceFile);
        GetStreamDBIndexAndSize();
        return;
    }

    // FileExporter Functions
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
    void FileExporter::PrintMatchesToCSV(std::vector<FileExportItem>& fileExportList) const
    {
        std::ofstream outputMatched("matched_tmp", std::ios::app);
        for (int i = 0; i < fileExportList.size(); i++)
        {
            FileExportItem& thisEntry = fileExportList[i];
            if (thisEntry.streamDBNumber == -1)
                continue;

            std::string output;
            output += "\"" + thisEntry.resourceFileName + "\",";
            output += std::to_string(thisEntry.streamDBIndex) + ",";
            output += std::to_string(thisEntry.streamDBFileOffset) + ",";
            output += std::to_string(thisEntry.streamDBSizeCompressed) + ",";
            output += std::to_string(thisEntry.streamDBSizeDecompressed) + ",";
            output += std::to_string(thisEntry.streamDBCompressionType) + ",";
            output += std::to_string(thisEntry.streamDBNumber) + ",";
            output += "\"" + thisEntry.streamDBFileName + "\"" + "\n";
            outputMatched.write(output.c_str(), output.length());
        }
        return;
    }
    void FileExporter::PrintUnmatchedToCSV(std::vector<FileExportItem>& fileExportList) const
    {
        std::ofstream outputUnmatched("unmatched_tmp", std::ios::app);
        for (int i = 0; i < fileExportList.size(); i++)
        {
            FileExportItem& thisEntry = fileExportList[i];
            if (thisEntry.streamDBNumber != -1)
                continue;

            std::string output;
            output += "\"" + thisEntry.resourceFileName + "\",";
            output += std::to_string(thisEntry.streamDBIndex) + ",";
            output += std::to_string(thisEntry.streamDBFileOffset) + ",";
            output += std::to_string(thisEntry.streamDBSizeCompressed) + ",";
            output += std::to_string(thisEntry.streamDBSizeDecompressed) + ",";
            output += std::to_string(thisEntry.streamDBCompressionType) + ",";
            output += std::to_string(thisEntry.streamDBNumber) + "\n";
            outputUnmatched.write(output.c_str(), output.length());
        }
        return;
    }

    // Constructs FileExporter, collects data needed for export.
    void FileExporter::Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles)
    {
        _TGAExportList = FileExportList(resourceFile, 21);
        std::vector<FileExportItem> tgaFilesToExport = _TGAExportList.GetFileExportItems();

        // Loop through streamdbData and decompress files
        for (int i = 0; i < tgaFilesToExport.size(); i++)
        {
            // First pass
            SearchStreamDBFilesForIndex(tgaFilesToExport[i], streamDBFiles);

            // Second pass for images that aren't found. 
            // This catches some wierd UI files whose hash indexes are off by 1.
            if (tgaFilesToExport[i].streamDBNumber == -1)
            {
                tgaFilesToExport[i].streamDBIndex--;
                SearchStreamDBFilesForIndex(tgaFilesToExport[i], streamDBFiles);
            }
        }
        
        // For debugging
        PrintMatchesToCSV(tgaFilesToExport);
        PrintUnmatchedToCSV(tgaFilesToExport);
        return;
    }
}
