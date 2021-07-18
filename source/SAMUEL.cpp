#include "SAMUEL.h"

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN
{
    // Helper function. Reads .streamdb file contents into SAMUEL. 
    void SAMUEL::readStreamDBEntries(std::vector<std::string> fileList)
    {
        for (auto i = fileList.begin(); i != fileList.end(); ++i)
        {
            // build filepath
            std::string filePath = _basePath + "\\" + *i;

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

    // Init function. Populates list of global .streamdb files from packagemapspec.json.
    void SAMUEL::SetGlobalStreamDBFileList()
    {
        _globalStreamDBFileList = packageMapSpec.GetFilesByResourceName(_basePath + "\\gameresources.resources");
        return;
    }

    // Init function. Reads packagemapspec.json data into SAMUEL. 
    void SAMUEL::LoadPackageMapSpec()
    {
        try
        {
            // read input filestream into stringstream
            std::ifstream inputStream = std::ifstream(_basePath + "\\packagemapspec.json");
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

    // Public API function. Reads .resource file data into SAMUEL. 
    void SAMUEL::LoadResource(std::string inputFile)
    {
        try 
        {
            resourceFile = ResourceFile(inputFile, 0);
            _resourceStreamDBFileList = packageMapSpec.GetFilesByResourceName(resourceFile.filename);
            return;
        }
        catch (...)
        {
            printf("ERROR: Failed to load resource file %s. \n", inputFile.c_str());
            return;
        }
    }

    // UNSORTED
    uint64_t getStreamDBIndex(uint64_t resourceId, int mipCount = -6)
    {
        // Get hex bytes string
        std::string hexBytes = int64ToHex(resourceId);

        // Reverse each byte
        for (int i = 0; i < hexBytes.size(); i += 2) {
            std::swap(hexBytes[i], hexBytes[i + 1i64]);
        }

        // Shift digits to the right
        hexBytes = hexBytes.substr(hexBytes.size() - 1) + hexBytes.substr(0, hexBytes.size() - 1);

        // Reverse each byte again
        for (int i = 0; i < hexBytes.size(); i += 2) {
            std::swap(hexBytes[i], hexBytes[i + 1i64]);
        }

        // Get second digit based on mip count
        hexBytes[1] = int64ToHex((char)(6i64 + mipCount))[1];

        // Convert hex string back to uint64 and return
        return hexToInt64(hexBytes);
    }
    byte* SAMUEL::getCompressedFileData(FILE& f, uint64 fileOffset, uint64 compressedSize)
    {
        byte* compressedData = NULL;
        compressedData = new byte[compressedSize];

        fseek(&f, (long)fileOffset, SEEK_SET);
        fread(compressedData, 1, compressedSize, &f);
        return compressedData;
    }
    TGAHeader SAMUEL::readTGAHeader(const char* tmpDecompressedHeader)
    {
        FILE* f;
        byte buff4[4];
        TGAHeader tgaHeader;

        if (fopen_s(&f, tmpDecompressedHeader, "rb") != 0)
        {
            printf("Error: failed to open %s for reading.\n", tmpDecompressedHeader);
            return tgaHeader;
        }

        if (f != NULL)
        {
            fseek(f, 59, SEEK_SET);
            fread(buff4, 4, 1, f);
            tgaHeader.numMips = *(uint32*)buff4;

            fseek(f, 20, SEEK_CUR);
            fread(buff4, 4, 1, f);
            tgaHeader.decompressedSize = *(uint32*)buff4;

            fread(buff4, 4, 1, f);
            tgaHeader.isCompressed = *(int*)buff4;

            fread(buff4, 4, 1, f);
            tgaHeader.compressedSize = *(uint32*)buff4;

            fclose(f);
        }
        return tgaHeader;
    }

    void SAMUEL::getStreamDBDataIndexes(ResourceFile& resourceFile)
    {
        FILE* f;
        if (fopen_s(&f, resourceFile.filename.c_str(), "rb") != 0)
        {
            printf("Error: failed to open %s for reading.\n", resourceFile.filename.c_str());
            return;
        }

        // Now, we need to iterate through .resource file entries
        for (uint64 i = 0; i < resourceFile.numFileEntries; i++)
        {
            ResourceEntry thisEntry = resourceFile.resourceEntries[i];

            // check version, for now we only want version = 21
            if (thisEntry.version != 21)
                continue;

            if (thisEntry.dataSizeCompressed == 0)
                continue;

            // FIXME
            if (thisEntry.compressionMode == 0)
                continue;

            // go to file offset and read compressed file header into memory
            byte* compressedData = getCompressedFileData(*f, thisEntry.dataOffset, thisEntry.dataSizeCompressed);
            if (!oodleDecompress("tmpTGAHeader", compressedData, thisEntry.dataSizeCompressed, thisEntry.dataSizeDecompressed))
                continue; // error

            compressedData = NULL;

            // get mip data from TGA
            TGAHeader tgaHeader = readTGAHeader("tmpTGAHeader");

            // get streamdbIndex
            endianSwap(thisEntry.hash);
            uint64 streamDBIndex = getStreamDBIndex(thisEntry.hash, tgaHeader.numMips);
            endianSwap(streamDBIndex);

            // populate streamdbData
            StreamDBData thisStreamDBFile;
            thisStreamDBFile.filename = thisEntry.name;
            thisStreamDBFile.streamDBIndex = streamDBIndex;
            thisStreamDBFile.streamDBSizeDecompressed = tgaHeader.decompressedSize;
            thisStreamDBFile.streamDBSizeCompressed = tgaHeader.compressedSize;

            if (tgaHeader.isCompressed == 1)
                thisStreamDBFile.streamDBCompressionType = thisEntry.compressionMode;

            streamDBData.push_back(thisStreamDBFile);
            continue;
        }

        if (f != NULL)
            fclose(f);

        return;
    }
    int SAMUEL::findMatchingIndex(uint64 streamDBIndex, int streamDBNumber)
    {
        for (int i = 0; i < streamDBFiles[streamDBNumber].indexEntryCount; i++)
        {
            if (streamDBFiles[streamDBNumber].indexEntryList[i].hashIndex != streamDBIndex)
                continue;

            return i;
        }
        return -1;
    }
    void SAMUEL::searchStreamDBFilesForIndex(StreamDBData& streamDBData)
    {
        for (int i = 0; i < streamDBFiles.size(); i++)
        {
            int matchIndex = findMatchingIndex(streamDBData.streamDBIndex, i);
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
    void SAMUEL::printMatchesToCSV()
    {
        // Loops through all items with match == 1 and write to file
        std::ofstream outputMatched("matched_tmp", std::ios::app);
        for (int i = 0; i < streamDBData.size(); i++)
        {
            StreamDBData& thisEntry = streamDBData[i];
            if (thisEntry.streamDBNumber == -1)
                continue;

            std::string output;
            output += "\"" + thisEntry.filename + "\",";
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
    void SAMUEL::printUnmatchedToCSV()
    {
        // Repeats the process, but for unmatched items (match == 0)
        std::ofstream outputUnmatched("unmatched_tmp", std::ios::app);
        for (int i = 0; i < streamDBData.size(); i++)
        {
            StreamDBData& thisEntry = streamDBData[i];
            if (thisEntry.streamDBNumber != -1)
                continue;

            std::string output;
            output += "\"" + thisEntry.filename + "\",";
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
}

int main(int argc, char* argv[])
{
    printf("SAMUEL v0.1 by SamPT \n");

    if (argc < 1) {
        printf("USAGE: SAMUEL.exe resourceFile basePath \n");
        return 1;
    }

    // Program Init
    SAMUEL SAM;
    SAM.SetBasePath(argv[2]);
    SAM.LoadPackageMapSpec();
    SAM.SetGlobalStreamDBFileList();

    // GUI should call this when user selects .resource file to load.
    SAM.LoadResource(argv[1]);

    // Decompress .resource file headers to get streamdb mipCount + streamdb Indexes
    SAM.getStreamDBDataIndexes(SAM.resourceFile);

    // Loop through streamdbData and decompress files
    for (int i = 0; i < SAM.streamDBData.size(); i++)
    {
        SAM.searchStreamDBFilesForIndex(SAM.streamDBData[i]);
    }

    SAM.printMatchesToCSV();
    SAM.printUnmatchedToCSV();
    return 0;
}