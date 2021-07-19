#include "SAMUEL.h"

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN
{
    // Init function. Reads packagemapspec.json data into SAMUEL. 
    void SAMUEL::LoadPackageMapSpec()
    {
        try
        {
            // read input filestream into stringstream
            std::ifstream inputStream = std::ifstream(_basePath + (char)fs::path::preferred_separator + "packagemapspec.json");
            std::stringstream strStream;
            strStream << inputStream.rdbuf();

            // convert to static string and call PackageMapSpec constructor.
            std::string jsonStream = strStream.str();
            packageMapSpec = PackageMapSpec(jsonStream);
            return;
        }
        catch (...)
        {
            printf("ERROR: Failed to load packagemapspec.json. \n");
            return;
        }
    }

    // Update SAMUEL's list of .streamdb files to search. Called by LoadResource().
    void SAMUEL::UpdateStreamDBFileList(std::string resourceFileName)
    {
        std::vector<std::string> appendList;
        appendList = packageMapSpec.GetFilesByResourceName(_basePath + (char)fs::path::preferred_separator + resourceFileName);

        // remove any files without .streamdb extension
        for (int i = 0; i < appendList.size(); i++)
        {
            size_t strPos = appendList[i].rfind(".streamdb");
            if (strPos != -1)
                continue;
            appendList.erase(appendList.begin() + i);
            i--;
        }

        // append to list, skip any files that were already added
        for (int i = 0; i < appendList.size(); i++)
        {
            auto it = std::find(_streamDBFileList.begin(), _streamDBFileList.end(), appendList[i]);
            if (it != _streamDBFileList.end())
                continue;
            _streamDBFileList.insert(std::end(_streamDBFileList), appendList[i]);
        }
        return;
    }
    
    // Read .streamdb contents into SAMUEL. Called by LoadResource().
    void SAMUEL::ReadStreamDBFiles()
    {
        for (auto i = _streamDBFileList.begin(); i != _streamDBFileList.end(); ++i)
        {
            // build filepath
            std::string filePath = _basePath + (char)fs::path::preferred_separator + *i;

            // make sure this is a .streamdb file before parsing
            size_t strPos = filePath.rfind(".streamdb");
            if (strPos == -1)
                continue;

            // read streamdb file into memory
            fs::path fsPath = filePath;
            StreamDBFile streamDBFile(fsPath);
            this->streamDBFiles.push_back(streamDBFile);
        }
    }

    // Decompress & Parse TGA Headers embedded in .resources file. Called by ExportAll().
    std::vector<EmbeddedTGAHeader> SAMUEL::ReadEmbeddedTGAHeaders(ResourceFile& resourceFile)
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
            byte* compressedData = resourceFile.GetCompressedFileHeader(*f, thisFile.resourceFileOffset, thisFile.resourceFileCompressedSize);

            // decompress with Oodle DLL
            if (!oodleDecompress("tmpTGAHeader", compressedData, thisFile.resourceFileCompressedSize, thisFile.resourceFileDecompressedSize))
                continue; // error

            // get mip data from TGA
            EmbeddedTGAHeader embeddedTGAHeader = resourceFile.ReadTGAHeader("tmpTGAHeader");
            embeddedTGAHeaders.push_back(embeddedTGAHeader);
        }

        fclose(f);
        return embeddedTGAHeaders;
    }

    // Helper function called by SearchStreamDBFilesForIndex().
    int SAMUEL::FindMatchingIndex(uint64 streamDBIndex, int streamDBNumber)
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
    void SAMUEL::SearchStreamDBFilesForIndex(FileExportItem& streamDBData)
    {
        for (int i = 0; i < streamDBFiles.size(); i++)
        {
            int matchIndex = FindMatchingIndex(streamDBData.streamDBIndex, i);
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
    uint64 SAMUEL::CalculateStreamDBIndex(uint64 resourceId, int mipCount)
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
    void SAMUEL::BuildFileExportList()
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

    // Debugging functions. Called by ExportAll().
    void SAMUEL::PrintMatchesToCSV()
    {
        // Loops through all items with match == 1 and write to file
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
    void SAMUEL::PrintUnmatchedToCSV()
    {
        // Repeats the process, but for unmatched items (match == 0)
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

    // Public API function. Reads .resource file data into SAMUEL. 
    void SAMUEL::LoadResource(std::string inputFile)
    {
        try
        {
            // load .resources file data
            resourceFile = ResourceFile(inputFile, 0);
        }
        catch (...)
        {
            printf("ERROR: Failed to read .resource file %s. \n", inputFile.c_str());
            return;
        }

        try
        {
            // get list of .streamdb files for this resource
            UpdateStreamDBFileList("gameresources.resources");      // globals
            UpdateStreamDBFileList(resourceFile.filename);          // resource-specific
        }
        catch (...)
        {
            printf("ERROR: Failed to read .streamdb list from packagemapspec.json. \n");
            return;
        }

        try
        {
            // load .streamdb data from files in _streamDBFileList
            ReadStreamDBFiles();
        }
        catch (...)
        {
            printf("ERROR: Failed to read .streamdb file data. \n");
            return;
        }
        return;
    }

    // Public API function. Exports all files from the currently loaded .resources.
    void SAMUEL::ExportAll()
    {
        // Build list of files to export
        BuildFileExportList();

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
            SearchStreamDBFilesForIndex(fileExportList[i]);
        }

        // For Debugging
        PrintMatchesToCSV();
        PrintUnmatchedToCSV();
        return;
    }
}

int main(int argc, char* argv[])
{
    printf("SAMUEL v0.1 by SamPT \n");

    if (argc == 1) {
        printf("USAGE: SAMUEL resourceFile basePath \n");
        return 1;
    }

    // Program Init
    SAMUEL SAM;
    SAM.SetBasePath(argv[2]);
    SAM.LoadPackageMapSpec();

    // API function - call this when user selects .resource file to load.
    SAM.LoadResource(argv[1]);

    // API function - call this when user clicks "Export All" button.
    SAM.ExportAll();

    return 0;
}