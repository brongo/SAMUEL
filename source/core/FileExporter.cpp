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
    void FileExportList::GetResourceEntries(const std::vector<ResourceEntry>& resourceData, std::vector<std::string> selectedFileNames, bool exportFromList)
    {
        // Iterate through currently loaded .resource entries to find supported files
        for (uint64 i = 0; i < resourceData.size(); i++)
        {
            ResourceEntry thisEntry = resourceData[i];

            // Can't export from empty list
            if (exportFromList && selectedFileNames.size() == 0)
                return;

            // each instance of FileExportList is for a different filetype. 21 = .tga, 31 = .md6mesh, 67 = .lwo, 0 = .decl
            if (thisEntry.Version != _FileType)
                continue;

            // only get selected file names, if list was provided
            if (exportFromList && selectedFileNames.size() > 0)
            {
                auto it = std::find(selectedFileNames.begin(), selectedFileNames.end(), thisEntry.Name);
                if (it == selectedFileNames.end())
                    continue;
            }

            // skips entries with no data to extract
            if (thisEntry.DataSize == 0)
                continue;

            // skip unsupported models
            if (thisEntry.Version == 67)
            {
                // skips world files (e.g. world_b403a83e93ccd372)
                if (thisEntry.Name.rfind("world_") != -1 && (thisEntry.Name.find("maps/game") != -1))
                    continue;

                // skips .bmodel files
                if (thisEntry.Name.rfind(".bmodel") != -1)
                    continue;
            }

            // skip unsupported md6
            if (thisEntry.Version == 31)
            {
                if (thisEntry.Name.rfind(".abc") != -1)
                    continue;
            }

            // skip unsupported images
            if (thisEntry.Version == 21)
            {                 
                // skip entries with "lightprobes" path
                if (thisEntry.Name.rfind("/lightprobes/") != -1)
                    continue;
            }

            // skip unsupported version 1 files
            if (thisEntry.Version == 1 && thisEntry.Type != "compfile")
                continue;

            // skip unsupported version 0 files
            if (thisEntry.Version == 0 && thisEntry.Type != "rs_streamfile")
                continue;

            FileExportItem exportItem;
            exportItem.resourceFileName = thisEntry.Name;
            exportItem.resourceFileOffset = thisEntry.DataOffset;
            exportItem.resourceFileCompressedSize = thisEntry.DataSize;
            exportItem.resourceFileDecompressedSize = thisEntry.DataSizeUncompressed;
            exportItem.resourceFileHash = thisEntry.StreamResourceHash;

            if (thisEntry.Version == 0 || thisEntry.Version == 1)
                exportItem.isStreamed = 0;

            _ExportItems.push_back(exportItem);
        }
        return;
    }
    void FileExportList::ParseEmbeddedFileHeaders(const std::string resourcePath)
    {
        // Open .resource file containing embedded file headers.
        FILE* f = fopen(resourcePath.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "Error: failed to open %s for reading.\n", resourcePath.c_str());
            return;
        }
        
        ResourceFileReader reader(resourcePath);

        // read compressed file headers into memory
        for (uint64 i = 0; i < _ExportItems.size(); i++)
        {
            FileExportItem& thisFile = _ExportItems[i];
            std::vector<byte> embeddedHeader = reader.GetEmbeddedFileHeader(f, thisFile.resourceFileOffset, thisFile.resourceFileCompressedSize);
            std::vector<byte> decompressedHeader = DecompressEmbeddedFileHeader(embeddedHeader, thisFile.resourceFileDecompressedSize);

            if (decompressedHeader.empty())
                continue; // decompression error
        
            if (_FileType == 21)
            {
                EmbeddedTGAHeader embeddedTGAHeader = reader.ReadTGAHeader(decompressedHeader);
                _TGAHeaderData.push_back(embeddedTGAHeader);
            }

            if (_FileType == 31)
            {
                EmbeddedMD6Header embeddedMD6Header = reader.ReadMD6Header(decompressedHeader);
                _MD6HeaderData.push_back(embeddedMD6Header);
            }

            if (_FileType == 67)
            {
                EmbeddedLWOHeader embeddedLWOHeader = reader.ReadLWOHeader(decompressedHeader);
                _LWOHeaderData.push_back(embeddedLWOHeader);
            }

            if (_FileType == 1)
            {
                EmbeddedCOMPFile embeddedCOMPFile = reader.ReadCOMPFile(decompressedHeader);
                _COMPFileData.push_back(embeddedCOMPFile);
            }

            if (_FileType == 0)
            {
                EmbeddedDECLFile embeddedDECLFile;
                embeddedDECLFile.unstreamedFileData = decompressedHeader;
                _DECLFileData.push_back(embeddedDECLFile);
            }


        }
        fclose(f);
        return;
    }
    void FileExportList::GetStreamDBIndexAndSize()
    {
        if (_FileType == 0 || _FileType == 1)
            return;

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
        if (_FileType == 0 || _FileType == 1)
            return;

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
    FileExportList::FileExportList(const std::string resourcePath, const std::vector<ResourceEntry>& resourceData, const std::vector<StreamDBFile>& streamDBFiles, int fileType, std::vector<std::string> selectedFileNames, bool exportFromList)
    {
        _FileType = fileType;
        GetResourceEntries(resourceData, selectedFileNames, exportFromList);
        ParseEmbeddedFileHeaders(resourcePath);
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
        size_t offset2 = _ResourceFilePath.find(".");

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
        fs::path outputPath = resourcePath.append(filePath);
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
    size_t FileExporter::CalculateRequiredDiskSpace()
    {
        size_t totalExportSize = 0;

        for (const auto& fileExport : _TGAExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        for (const auto& fileExport : _MD6ExportList.GetFileExportItems())
            totalExportSize += fileExport.streamDBSizeDecompressed;

        for (const auto& fileExport : _DECLExportList.GetFileExportItems())
            totalExportSize += fileExport.resourceFileDecompressedSize;

        // Disabling for now, unpredictable
        // for (const auto& fileExport : _LWOExportList.GetFileExportItems())
        //    totalExportSize += fileExport.streamDBSizeDecompressed;

        return totalExportSize;
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
        else if (fileType == "LWO")
            fileExportList = &_LWOExportList;
        else if (fileType == "DECL")
            fileExportList = &_DECLExportList;
        else if (fileType == "COMPFILE")
            fileExportList = &_COMPExportList;
        else
            return;

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

            if (fileType == "TGA")
            {
                if (thisFile.isStreamed == 0)
                    fileData = fileExportList->GetTGAFileData(i);

                DDSHeaderBuilder ddsBuilder(thisFile.tgaPixelWidth, thisFile.tgaPixelHeight, thisFile.streamDBSizeDecompressed, static_cast<ImageType>(thisFile.tgaImageType));
                std::vector<byte> ddsFileHeader = ddsBuilder.ConvertToByteVector();
                
                // write to filesystem
                thisFile.resourceFileName += ".dds";
                fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
                WriteFileToDisk(fileExportList, fullPath, fileData, ddsFileHeader);
                continue;
            }

            if (fileType == "DECL")
            {
                fileData = fileExportList->GetDECLFileData(i);
                fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
                WriteFileToDisk(fileExportList, fullPath, fileData);
                continue;
            }

            if (fileType == "LWO")
            {
                std::vector<byte> lwoFileHeader = fileExportList->GetLWOFileHeader(i);
                fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
                WriteFileToDisk(fileExportList, fullPath, fileData, lwoFileHeader);
                continue;
            }

            if (fileType == "MD6")
            {
                std::vector<byte> md6FileHeader = fileExportList->GetMD6FileHeader(i);
                fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
                WriteFileToDisk(fileExportList, fullPath, fileData, md6FileHeader);
                continue;
            }

            if (fileType == "COMPFILE")
            {
                int decompressedSize = 0;
                decompressedSize = fileExportList->GetCOMPFileDecompressedSize(i);
                fileData = fileExportList->GetCOMPFileData(i);

                fileData = oodleDecompress(fileData, decompressedSize);
                if (fileData.empty())
                {
                    fprintf(stderr, "Error: Failed to decompress: %s \n", thisFile.resourceFileName.c_str());
                    fileExportList->errorCount++;
                    continue;
                }

                fs::path fullPath = BuildOutputPath(thisFile.resourceFileName);
                WriteFileToDisk(fileExportList, fullPath, fileData);
                continue;
            }
        }
        printf("Wrote %llu %s files, %d not found, %d errors. \n", fileExportItems.size(), fileType.c_str(), fileExportList->notFound, fileExportList->errorCount);
        return;
    }

    // Constructs FileExporter, collects data needed for export.
    void FileExporter::Init(std::string resourcePath, const std::vector<ResourceEntry>& resourceData, const std::vector<StreamDBFile>& streamDBFiles, const std::string outputDirectory)
    {
        _OutDir = outputDirectory;
        _ResourceFilePath = resourcePath;
        _TGAExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 21);
        _MD6ExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 31);
        _LWOExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 67);
        _DECLExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 0);
        _COMPExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 1);

        // get total size of files to export
        _TotalExportSize = CalculateRequiredDiskSpace();
        return;
    }
    void FileExporter::InitFromList(std::string resourcePath, const std::vector<ResourceEntry>& resourceData, const std::vector<StreamDBFile>& streamDBFiles, const std::string outputDirectory, const std::vector<std::vector<std::string>> userSelectedFileList)
    {
        _OutDir = outputDirectory;
        _ResourceFilePath = resourcePath;
        std::vector<std::string> userSelectedTGAFiles;
        std::vector<std::string> userSelectedMD6Files;
        std::vector<std::string> userSelectedLWOFiles;
        std::vector<std::string> userSelectedDECLFiles;
        std::vector<std::string> userSelectedCOMPFiles;

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
                case 0:
                    userSelectedDECLFiles.push_back(userSelectedFileList[i][0]);
                    break;
                case 1:
                    userSelectedCOMPFiles.push_back(userSelectedFileList[i][0]);
                    break;
                default:
                    break;
            }
        }
        
        _TGAExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 21, userSelectedTGAFiles, 1);
        _MD6ExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 31, userSelectedMD6Files, 1);
        _LWOExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 67, userSelectedLWOFiles, 1);
        _DECLExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 0, userSelectedDECLFiles, 1);
        _COMPExportList = FileExportList(_ResourceFilePath, resourceData, streamDBFiles, 1, userSelectedCOMPFiles, 1);

        // get total size of files to export
        _TotalExportSize = CalculateRequiredDiskSpace();
        return;
    }
}
