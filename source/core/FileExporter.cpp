#include "FileExporter.h"

namespace HAYDEN
{
    // FileExportList - Subroutines
    std::vector<byte> FileExportList::DecompressEmbeddedFileHeader(std::vector<byte> embeddedHeader, const uint64 decompressedSize)
    {
        if (embeddedHeader.size() != decompressedSize)
        {
            std::vector<byte> decompressedHeader = oodleDecompress(embeddedHeader, decompressedSize);
            return decompressedHeader;
        }
        return embeddedHeader;
    }
    uint64 FileExportList::CalculateStreamDBIndex(const uint64 resourceId, const int mipCount) const
    {
        // Get hex bytes string
        std::string hexBytes = intToHex(resourceId);

        // Reverse each byte
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64)1]);

        // Shift digits to the right
        hexBytes = hexBytes.substr(hexBytes.size() - 1) + hexBytes.substr(0, hexBytes.size() - 1);

        // Reverse each byte again
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64)1]);

        // Get second digit based on mip count
        hexBytes[1] = intToHex((char)(6 + mipCount))[1];

        // Convert hex string back to uint64 and return
        return hexToInt64(hexBytes);
    }

    // FileExportList - Helper functions for FileExportList constructor
    void FileExportList::GetResourceEntries(const ResourceFile& resourceFile, std::vector<std::string> selectedFileNames, bool exportFromList)
    {
        // Iterate through currently loaded .resource entries to find supported files
        for (uint64 i = 0; i < resourceFile.numFileEntries; i++)
        {
            ResourceEntry thisEntry = resourceFile.resourceEntries[i];

            // Can't export from empty list
            if (exportFromList && selectedFileNames.size() == 0)
                return;

            // each instance of FileExportList is for a different filetype. 21 = .tga, 31 = .md6mesh
            if (thisEntry.version != _FileType)
                continue;

            // only get selected file names, if list was provided
            if (exportFromList && selectedFileNames.size() > 0)
            {
                auto it = std::find(selectedFileNames.begin(), selectedFileNames.end(), thisEntry.name);
                if (it == selectedFileNames.end())
                    continue;
            }

            // skips entries with no data to extract
            if (thisEntry.dataSizeCompressed == 0)
                continue;

            // skip unsupported images
            if (thisEntry.version == 21)
            {                 
                // skip entries with "lightprobes" path
                if (thisEntry.name.rfind("/lightprobes/") != -1)
                    continue;
            }

            // skip unsupported models
            if (thisEntry.version == 67)
            {
                // skips world files (e.g. world_b403a83e93ccd372)
                if (thisEntry.name.rfind("world_") != -1 && (thisEntry.name.find("maps/game") != -1))
                    continue;

                // skips .bmodel files
                if (thisEntry.name.rfind(".bmodel") != -1)
                    continue;
            }

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
    void FileExportList::ParseEmbeddedFileHeaders(const ResourceFile& resourceFile)
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
            std::vector<byte> embeddedHeader = resourceFile.GetEmbeddedFileHeader(f, thisFile.resourceFileOffset, thisFile.resourceFileCompressedSize);
            std::vector<byte> decompressedHeader = DecompressEmbeddedFileHeader(embeddedHeader, thisFile.resourceFileDecompressedSize);

            if (decompressedHeader.empty())
                continue; // decompression error
        
            if (_FileType == 21)
            {
                EmbeddedTGAHeader embeddedTGAHeader = resourceFile.ReadTGAHeader(decompressedHeader);
                _TGAHeaderData.push_back(embeddedTGAHeader);
            }

            if (_FileType == 31)
            {
                EmbeddedMD6Header embeddedMD6Header = resourceFile.ReadMD6Header(decompressedHeader);
                _MD6HeaderData.push_back(embeddedMD6Header);
            }

            if (_FileType == 67)
            {
                EmbeddedLWOHeader embeddedLWOHeader = resourceFile.ReadLWOHeader(decompressedHeader);
                _LWOHeaderData.push_back(embeddedLWOHeader);
            }
        }
        fclose(f);
        return;
    }
    void FileExportList::GetStreamDBIndexAndSize()
    {
        for (int i = 0; i < _ExportItems.size(); i++)
        {
            FileExportItem& thisFile = _ExportItems[i];
            int numMips = -6;

            // tga data
            if (_FileType == 21)
            {              
                numMips = _TGAHeaderData[i].streamDBMipCount;
                thisFile.streamDBSizeDecompressed = _TGAHeaderData[i].decompressedSize;
                thisFile.streamDBSizeCompressed = _TGAHeaderData[i].compressedSize;
                thisFile.tgaPixelHeight = _TGAHeaderData[i].pixelHeight;
                thisFile.tgaPixelWidth = _TGAHeaderData[i].pixelWidth;
                thisFile.tgaImageType = _TGAHeaderData[i].imageType;

                if (_TGAHeaderData[i].isCompressed == 1)
                    thisFile.streamDBCompressionType = 2;

                if (_TGAHeaderData[i].isStreamed == 0 && _TGAHeaderData[i].isCompressed == 0)
                    thisFile.isStreamed = 0;
            }
               
            // md6mesh data
            if (_FileType == 31)
            {
                thisFile.streamDBSizeDecompressed = _MD6HeaderData[i].decompressedSize;
                thisFile.streamDBSizeCompressed = _MD6HeaderData[i].compressedSize;
                thisFile.streamDBCompressionType = 2;
            }

            // lwo data
            if (_FileType == 67)
            {
                thisFile.streamDBSizeDecompressed = _LWOHeaderData[i].decompressedSize;
                thisFile.streamDBSizeCompressed = _LWOHeaderData[i].compressedSize;
                thisFile.streamDBCompressionType = 2;
            }

            // Convert resourceID to streamDBIndex
            endianSwap(thisFile.resourceFileHash);
            uint64 streamDBIndex = CalculateStreamDBIndex(thisFile.resourceFileHash, numMips);
            endianSwap(streamDBIndex);
            thisFile.streamDBIndex = streamDBIndex;
        }
        return;
    }
    void FileExportList::GetStreamDBFileOffsets(const std::vector<StreamDBFile>& streamDBFiles)
    {
        for (int i = 0; i < _ExportItems.size(); i++)
        {
            if (_ExportItems[i].isStreamed == 0)
                continue;
                
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
    FileExportList::FileExportList(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, int fileType, std::vector<std::string> selectedFileNames, bool exportFromList)
    {
        _FileType = fileType;
        GetResourceEntries(resourceFile, selectedFileNames, exportFromList);
        ParseEmbeddedFileHeaders(resourceFile);
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
    std::string FileExporter::GetResourceFolder()
    {
        std::string resourceFolder;
        size_t offset1 = _ResourceFilePath.rfind("/");
        size_t offset2 = _ResourceFilePath.rfind(".");

        // this shouldn't happen, but if it somehow does, we just won't use the resourceFolder in our export path.
        if (offset1 < 8 || offset2 < offset1)
            return resourceFolder;

        // get only the resource folder name, without path or file extension
        size_t length = offset2 - offset1;
        resourceFolder = _ResourceFilePath.substr(offset1 + 1, length - 1);

        // separates dlc hub from regular hub folder
        if (_ResourceFilePath.substr(offset1 - 8, 8) == "/dlc/hub")
            resourceFolder = "dlc_" + resourceFolder;

        return resourceFolder;
    }
    fs::path FileExporter::BuildOutputPath(const std::string filePath)
    {
        std::string resourceFolder = GetResourceFolder();
        fs::path resourcePath = _OutDir + (char)fs::path::preferred_separator + resourceFolder;
        fs::path outputPath = resourcePath / fs::path(filePath);
        outputPath.make_preferred();
        return outputPath;
    }
    void FileExporter::WriteFileToDisk(FileExportList* fileExportList, const fs::path& fullPath, const std::vector<byte>& fileData, const std::vector<byte>& headerData)
    {
        fs::path folderPath = fullPath;
        folderPath.remove_filename();

        // create output directories if needed
        if (!fs::exists(folderPath))
        {
            if (!mkpath(folderPath))
            {
                fprintf(stderr, "Error: Failed to create directories for file: %s \n", fullPath.string().c_str());
                fileExportList->errorCount++;
                return;
            }
        }

        // open file for writing
        #ifdef _WIN32
        FILE* outFile = openLongFilePathWin32(fullPath); //wb
        #else
        FILE* outFile = fopen(fullPath.string().c_str(), "wb");
        #endif

        if (outFile == NULL)
        {
            fprintf(stderr, "Error: Failed to open file for writing: %s \n", fullPath.string().c_str());
            fileExportList->errorCount++;
            return;
        }

        // optional parameter, used for generated DDS headers
        if (headerData.size() > 0)
            fwrite(headerData.data(), headerData.size(), 1, outFile);

        // write actual file contents
        fwrite(fileData.data(), fileData.size(), 1, outFile);
        fclose(outFile);
        return;
    }

    // FileExporter - Debug Functions
    void FileExporter::PrintMatchesToCSV(std::vector<FileExportItem>& fileExportList) const
    {
        std::ofstream outputMatched("matched.csv", std::ios::app);
        for (int i = 0; i < fileExportList.size(); i++)
        {
            FileExportItem& thisEntry = fileExportList[i];
            if (thisEntry.streamDBNumber == -1 && thisEntry.isStreamed == 1)
                continue;

            std::string output;
            output += "\"" + thisEntry.resourceFileName + "\",";
            //output += std::to_string(thisEntry.streamDBIndex) + ",";
            //output += std::to_string(thisEntry.streamDBFileOffset) + ",";
            output += std::to_string(thisEntry.streamDBSizeCompressed) + ",";
            output += std::to_string(thisEntry.streamDBSizeDecompressed) + ",";
            output += std::to_string(thisEntry.streamDBCompressionType) + ",";
            output += std::to_string(thisEntry.tgaImageType) + ",";
            output += std::to_string(thisEntry.tgaPixelWidth) + ",";
            output += std::to_string(thisEntry.tgaPixelHeight) + "," + "\n";
            //output += std::to_string(thisEntry.streamDBNumber) + ",";
            //output += "\"" + thisEntry.streamDBFileName + "\"" + "\n";
            outputMatched.write(output.c_str(), output.length());
        }
        return;
    }
    void FileExporter::PrintUnmatchedToCSV(std::vector<FileExportItem>& fileExportList) const
    {
        std::ofstream outputUnmatched("unmatched.csv", std::ios::app);
        for (int i = 0; i < fileExportList.size(); i++)
        {
            FileExportItem& thisEntry = fileExportList[i];
            if (thisEntry.streamDBNumber != -1 || thisEntry.isStreamed == 0)
                continue;

            std::string output;
            output += "\"" + thisEntry.resourceFileName + "\",";
            //output += std::to_string(thisEntry.streamDBIndex) + ",";
            //output += std::to_string(thisEntry.streamDBFileOffset) + ",";
            output += std::to_string(thisEntry.streamDBSizeCompressed) + ",";
            output += std::to_string(thisEntry.streamDBSizeDecompressed) + ",";
            output += std::to_string(thisEntry.streamDBCompressionType) + ",";
            output += std::to_string(thisEntry.tgaImageType) + ",";
            output += std::to_string(thisEntry.tgaPixelWidth) + ",";
            output += std::to_string(thisEntry.tgaPixelHeight) + "," + "\n";
            //output += std::to_string(thisEntry.streamDBNumber) + ",";
            //output += "\"" + thisEntry.streamDBFileName + "\"" + "\n";
            outputUnmatched.write(output.c_str(), output.length());
        }
        return;
    }

    // FileExporter - Export Functions
    void FileExporter::ExportFiles(const std::vector<StreamDBFile>& streamDBFiles, std::string fileType)
    {
        FileExportList* fileExportList = NULL;
        std::vector<FileExportItem> fileExportItems;

        if (fileType == "TGA")
            fileExportList = &_TGAExportList;
        else if (fileType == "MD6")
            fileExportList = &_MD6ExportList;
        else
            fileExportList = &_LWOExportList;

        fileExportItems = fileExportList->GetFileExportItems(); 

        for (int i = 0; i < fileExportItems.size(); i++)
        {
            std::vector<byte> fileData;
            FileExportItem& thisFile = fileExportItems[i];

            // Don't try to export resources that weren't found
            if (thisFile.streamDBNumber == -1 && thisFile.isStreamed == 1)
            {
                fileExportList->notFound++;
                continue;
            }

            // Get data from .streamdb, decompress if needed.
            if (thisFile.isStreamed == 1)
            {
                const StreamDBFile& thisStreamDBFile = streamDBFiles[thisFile.streamDBNumber];
                fileData = GetBinaryFileFromStreamDB(thisFile, thisStreamDBFile);
                
                // check if compressed
                if (thisFile.streamDBSizeCompressed != thisFile.streamDBSizeDecompressed)
                {
                    fileData = oodleDecompress(fileData, thisFile.streamDBSizeDecompressed);
                    if (fileData.empty())
                    {
                        fprintf(stderr, "Error: Failed to decompress: %s \n", thisFile.resourceFileName.c_str());
                        fileExportList->errorCount++;
                        continue;
                    }
                }
            }

            // TGA files only
            if (fileType == "TGA")
            {
                if (thisFile.isStreamed == 0)
                    fileData = fileExportList->GetTGAFileData(i);

                DDSHeaderBuilder ddsBuilder(thisFile.tgaPixelWidth, thisFile.tgaPixelHeight, thisFile.streamDBSizeDecompressed, static_cast<ImageType>(thisFile.tgaImageType));
                std::vector<byte> ddsFileHeader = ddsBuilder.ConvertToByteVector();
                
                // write to filesystem
                fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
                WriteFileToDisk(fileExportList, fullPath, fileData, ddsFileHeader);
                continue;
            }

            // write to filesystem
            fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
            WriteFileToDisk(fileExportList, fullPath, fileData);        
        }
        printf("Wrote %llu %s files, %d not found, %d errors. \n", fileExportItems.size(), fileType.c_str(), fileExportList->notFound, fileExportList->errorCount);
        return;
    }

    // Constructs FileExporter, collects data needed for export.
    void FileExporter::Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, const std::string outputDirectory)
    {
        _OutDir = outputDirectory;
        _ResourceFilePath = resourceFile.filename;

        _TGAExportList = FileExportList(resourceFile, streamDBFiles, 21);
        _MD6ExportList = FileExportList(resourceFile, streamDBFiles, 31);
        _LWOExportList = FileExportList(resourceFile, streamDBFiles, 67);
        std::vector<FileExportItem> tgaFilesToExport = _TGAExportList.GetFileExportItems();
        std::vector<FileExportItem> md6FilesToExport = _MD6ExportList.GetFileExportItems();
        std::vector<FileExportItem> lwoFilesToExport = _LWOExportList.GetFileExportItems();

        // debug tga export
        // PrintMatchesToCSV(tgaFilesToExport);
        // PrintUnmatchedToCSV(tgaFilesToExport);

        // debug md6 export
        // PrintMatchesToCSV(md6FilesToExport);
        // PrintUnmatchedToCSV(md6FilesToExport);

        // debug lwo export
        // PrintMatchesToCSV(lwoFilesToExport);
        // PrintUnmatchedToCSV(lwoFilesToExport);

        // get total size of files to export
        size_t totalExportSize = 0;

        for (const auto& fileExport : _TGAExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        for (const auto& fileExport : _MD6ExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        for (const auto& fileExport : _LWOExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        if (fs::space(fs::current_path()).available < totalExportSize)
        {
            fprintf(stderr, "Error: Not enough space in disk.\n");
            fprintf(stderr, "Exporting from this file requires at least %.2lf MB of free space.\n", (double)totalExportSize / (1024 * 1024));
            exit(1);
        }

        return;
    }
    void FileExporter::InitFromList(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, const std::string outputDirectory, const std::vector<std::vector<std::string>> userSelectedFileList)
    {
        _OutDir = outputDirectory;
        _ResourceFilePath = resourceFile.filename;

        std::vector<std::string> userSelectedTGAFiles;
        std::vector<std::string> userSelectedMD6Files;
        std::vector<std::string> userSelectedLWOFiles;

        // build separate lists for each file type.
        for (int i = 0; i < userSelectedFileList.size(); i++)
        {
            int fileType = std::stoi(userSelectedFileList[i][2]);
            switch (fileType)
            {
                case 21:
                    userSelectedTGAFiles.push_back(userSelectedFileList[i][0]);
                    break;
                case 31:
                    userSelectedMD6Files.push_back(userSelectedFileList[i][0]);
                    break;
                case 67:
                    userSelectedLWOFiles.push_back(userSelectedFileList[i][0]);
                    break;
                default:
                    break;
            }
        }
        
        _TGAExportList = FileExportList(resourceFile, streamDBFiles, 21, userSelectedTGAFiles, 1);
        _MD6ExportList = FileExportList(resourceFile, streamDBFiles, 31, userSelectedMD6Files, 1);
        _LWOExportList = FileExportList(resourceFile, streamDBFiles, 67, userSelectedLWOFiles, 1);
        std::vector<FileExportItem> tgaFilesToExport = _TGAExportList.GetFileExportItems();
        std::vector<FileExportItem> md6FilesToExport = _MD6ExportList.GetFileExportItems();
        std::vector<FileExportItem> lwoFilesToExport = _LWOExportList.GetFileExportItems();

        // get total size of files to export
        size_t totalExportSize = 0;

        for (const auto& fileExport : _TGAExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        for (const auto& fileExport : _MD6ExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        for (const auto& fileExport : _LWOExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        if (fs::space(fs::current_path()).available < totalExportSize)
        {
            fprintf(stderr, "Error: Not enough space in disk.\n");
            fprintf(stderr, "Exporting from this file requires at least %.2lf MB of free space.\n", (double)totalExportSize / (1024 * 1024));
            exit(1);
        }
        return;
    }
}