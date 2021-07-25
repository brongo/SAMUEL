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
                auto decompressedData = oodleDecompress(compressedData, thisFile.resourceFileDecompressedSize);
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
    void FileExportList::GetStreamDBFileOffsets(const std::vector<StreamDBFile>& streamDBFiles)
    {
        for (int i = 0; i < _ExportItems.size(); i++)
        {
            // Searches every .streamdb file for a matching streamDBIndex
            FileExportItem& thisFile = _ExportItems[i];
            for (int j = 0; j < streamDBFiles.size(); j++)
            {
                uint64 fileOffset = streamDBFiles[j].GetFileOffsetInStreamDB(thisFile.streamDBIndex, thisFile.streamDBSizeCompressed);
                
                // index matched, size too small, increment index by 1 and check again.
                if (fileOffset == -2)
                    fileOffset = streamDBFiles[j].GetFileOffsetInStreamDB(thisFile.streamDBIndex + 1, thisFile.streamDBSizeCompressed);
                
                if (fileOffset == -1)
                    continue;

                thisFile.streamDBNumber = j;
                thisFile.streamDBFileName = streamDBFiles[j].fileName;
                thisFile.streamDBFileOffset = fileOffset;
                break;
            }

            // match found, go to next file
            if (thisFile.streamDBNumber != -1)
                continue;

            // Second pass for images that aren't found. 
            // This catches some wierd UI files whose streamDBIndex is off by 1.
            thisFile.streamDBIndex--;
            for (int j = 0; j < streamDBFiles.size(); j++)
            {
                uint64 fileOffset = streamDBFiles[j].GetFileOffsetInStreamDB(thisFile.streamDBIndex, thisFile.streamDBSizeCompressed);
                if (fileOffset == -1)
                    continue;

                thisFile.streamDBNumber = j;
                thisFile.streamDBFileName = streamDBFiles[j].fileName;
                thisFile.streamDBFileOffset = fileOffset;
                break;
            }
        }
        return;
    }

    // Constructs FileExportList
    FileExportList::FileExportList(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, int fileType)
    {
        _FileType = fileType;
        _ResourceFileName = resourceFile.filename;
        GetResourceEntries(resourceFile);
        ParseEmbeddedTGAHeaders(resourceFile);
        GetStreamDBIndexAndSize();
        GetStreamDBFileOffsets(streamDBFiles);
        return;
    }

    // FileExporter - Subroutines
    std::vector<byte> FileExporter::GetBinaryFileFromStreamDB(const FileExportItem& fileExportInfo, const StreamDBFile& streamDBFile)
    {
        std::ifstream f;
        f.open(fileExportInfo.streamDBFileName, std::ios::in | std::ios::binary);
        auto fileData = streamDBFile.GetEmbeddedFile(f, fileExportInfo.streamDBFileOffset, fileExportInfo.streamDBSizeCompressed);
        f.close();

        return fileData;
    }
    std::vector<byte> FileExporter::ConstructDDSFileHeader()
    {
        DDSHeader defaultDDS;
        auto ptr = reinterpret_cast<byte*>(&defaultDDS);
        auto buffer = std::vector<byte>(ptr, ptr + sizeof(defaultDDS));
        return buffer;
    }
    fs::path FileExporter::BuildOutputPath(const std::string filePath)
    {
        fs::path outputPath = _OutDir / fs::path(filePath);
        outputPath.make_preferred();
        return outputPath;
    }

    // FileExporter - Debug Functions
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

    // FileExporter - Export Functions
    void FileExporter::ExportTGAFiles(const std::vector<StreamDBFile>& streamDBFiles)
    {
        std::vector<FileExportItem> tgaFilesToExport = _TGAExportList.GetFileExportItems();
        for (int i = 0; i < tgaFilesToExport.size(); i++)
        {
            // Don't try to export resources that weren't found
            if (tgaFilesToExport[i].streamDBNumber == -1)
                continue;

            FileExportItem& thisFile = tgaFilesToExport[i];
            const StreamDBFile& thisStreamDBFile = streamDBFiles[thisFile.streamDBNumber];

            // get binary file data from streamdb
            std::vector<byte> fileData = GetBinaryFileFromStreamDB(thisFile, thisStreamDBFile);

            // check if decompression is necessary
            if (thisFile.streamDBSizeCompressed != thisFile.streamDBSizeDecompressed)
            {
                fileData = oodleDecompress(fileData, thisFile.streamDBSizeDecompressed);
                if (fileData.empty())
                    continue; // error
            }

            // construct DDS file header
            // std::vector<byte> ddsFileHeader = ConstructDDSFileHeader(); 

            // parse filepath and create folders if necessary
            fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
            fs::path folderPath = fullPath;
            folderPath.remove_filename();

            if (!fs::exists(folderPath))
            {
                if (!fs::create_directories(folderPath));
                    continue;  // error: failed to create output directories
            }
            
            FILE* outFile = fopen(fullPath.string().c_str(), "wb");
            if (outFile == NULL)
                continue; // error

            // fwrite(ddsFileHeader.data(), ddsFileHeader.size(), 1, outFile);
            fwrite(fileData.data(), fileData.size(), 1, outFile);
            fclose(outFile);
            printf("Test");
            continue;
        }
        return;
    }

    // Constructs FileExporter, collects data needed for export.
    void FileExporter::Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, std::string outputDirectory)
    {
        _OutDir = outputDirectory;
        _TGAExportList = FileExportList(resourceFile, streamDBFiles, 21);
        std::vector<FileExportItem> tgaFilesToExport = _TGAExportList.GetFileExportItems();
 
        // For debugging
        PrintMatchesToCSV(tgaFilesToExport);
        PrintUnmatchedToCSV(tgaFilesToExport);
        return;
    }
}
