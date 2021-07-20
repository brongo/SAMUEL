#include "SAMUEL.h"

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN
{
    // Reads packagemapspec.json data into SAMUEL. Called by Init().
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
            _PackageMapSpec = PackageMapSpec(jsonStream);
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
        appendList = _PackageMapSpec.GetFilesByResourceName(_basePath + (char)fs::path::preferred_separator + resourceFileName);

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
            auto it = std::find(_StreamDBFileList.begin(), _StreamDBFileList.end(), appendList[i]);
            if (it != _StreamDBFileList.end())
                continue;
            _StreamDBFileList.insert(std::end(_StreamDBFileList), appendList[i]);
        }
        return;
    }
    
    // Read .streamdb file data into SAMUEL. Called by LoadResource().
    void SAMUEL::ReadStreamDBFiles()
    {
        for (auto i = _StreamDBFileList.begin(); i != _StreamDBFileList.end(); ++i)
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
            _StreamDBFileData.push_back(streamDBFile);
        }
    }

    // Debugging functions.
    void SAMUEL::PrintMatchesToCSV()
    {
        std::vector<FileExportItem> fileExportList = _Exporter.GetFileExportList();
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
        std::vector<FileExportItem> fileExportList = _Exporter.GetFileExportList();
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

    // Public API functions - SAMUEL
    void SAMUEL::LoadResource(std::string inputFile)
    {
        try
        {
            // load .resources file data
            _ResourceFile = ResourceFile(inputFile, 0);
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
            UpdateStreamDBFileList(_ResourceFile.filename);          // resource-specific
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
    void SAMUEL::ExportAll()
    {
        // Initialize file exporter
        _Exporter.Init(_ResourceFile, _StreamDBFileData);

        // For Debugging
        PrintMatchesToCSV();
        PrintUnmatchedToCSV();
        return;
    }
    void SAMUEL::Init(std::string basePath)
    {
        SetBasePath(basePath);
        LoadPackageMapSpec();
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
    SAM.Init(argv[2]);

    // API function - call this when user selects .resource file to load.
    SAM.LoadResource(argv[1]);

    // API function - call this when user clicks "Export All" button.
    SAM.ExportAll();

    return 0;
}