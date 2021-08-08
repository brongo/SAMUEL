#include "SAMUEL.h"

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN
{
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
            fprintf(stderr, "Error: Failed to load packagemapspec.json.\n");
            return;
        }
    }
    void SAMUEL::UpdateStreamDBFileList(const std::string resourceFileName)
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

    // Public API functions - SAMUEL
    void SAMUEL::LoadResource(const std::string inputFile)
    {
        try
        {
            // load .resources file data
            _ResourceFile = ResourceFile(inputFile, 0);
        }
        catch (...)
        {
            fprintf(stderr, "Error: Failed to read .resource file %s.\n", inputFile.c_str());
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
            fprintf(stderr, "Error: Failed to read .streamdb list from packagemapspec.json.\n");
            return;
        }

        try
        {
            // load .streamdb data from files in _streamDBFileList
            ReadStreamDBFiles();
        }
        catch (...)
        {
            fprintf(stderr, "Error: Failed to read .streamdb file data.\n");
            return;
        }
        return;
    }
    void SAMUEL::ExportAll(std::string outputDirectory)
    {
        _Exporter.Init(_ResourceFile, _StreamDBFileData, outputDirectory);
        _Exporter.ExportTGAFiles(_StreamDBFileData);
        // _Exporter.ExportMD6Files(_StreamDBFileData);
        // _Exporter.ExportLWOFiles(_StreamDBFileData);
        return;
    }
    void SAMUEL::Init(const std::string basePath)
    {
        if (!fs::exists("oo2core_8_win64.dll"))
        {
            fprintf(stderr, "Error: Could not find oo2core_8_win64.dll in the current directory.\n");
            exit(1);
        }

#ifdef __linux__
        if (!fs::exists("liblinoodle.so"))
        {
            fprintf(stderr, "Error: Could not find liblinoodle.so in the current directory.\n");
            exit(1);
        }
#endif

        SetBasePath(basePath);
        LoadPackageMapSpec();
    }
}

/*
int main(int argc, char* argv[])
{
    printf("SAMUEL v0.1 by SamPT\n");

    if (argc < 2)
    {
        printf("USAGE: SAMUEL /path/to/resourceFile\n");
        return 1;
    }

    // Get base path from resource path
    std::string resourcePath(argv[1]);
    auto baseIndex = resourcePath.find("base");
    if (baseIndex == -1)
    {
        fprintf(stderr, "Error: Failed to get game's base path.\n");
    }
    std::string basePath = resourcePath.substr(0, baseIndex + 4);

    // Get export path from argv[0]
    std::string exportPath = fs::absolute(argv[0]).replace_filename("exports").string();

    SAMUEL SAM;
    SAM.Init(basePath);
    SAM.LoadResource(resourcePath);
    SAM.ExportAll(exportPath);
    return 0;
}
*/
