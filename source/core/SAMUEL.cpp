#include "SAMUEL.h"

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN
{
    // Private functions - SAMUEL
    void SAMUEL::ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail)
    {
        _LastErrorMessage = errorMessage;
        _LastErrorDetail = errorDetail;

        if (isFatal)
            _HasFatalError = 1;

        std::string consoleMsg = _LastErrorMessage + " " + _LastErrorDetail + "\n";
        fprintf(stderr, "%s", consoleMsg.c_str());
        return;
    }
    bool SAMUEL::FindOodleDLL()
    {
        if (!fs::exists("oo2core_8_win64.dll"))
        {
            ThrowError(1, "SAMUEL requires you to manually copy oo2core_8_win64.dll from Doom Eternal's installation folder and place it in the same folder as SAMUEL.exe. Please do this, and then run SAMUEL.exe again.");
            return 0;
        }

        #ifdef __linux__
        if (!fs::exists("liblinoodle.so"))
        {
            ThrowError(1, "SAMUEL requires you to manually copy liblinoodle.so from Doom Eternal's \"base\" folder and place it in the same directory as SAMUEL. Please do this, and then run the SAMUEL program again.");
            return 0;
        }
        #endif

        return 1;
    };
    bool SAMUEL::FindBasePath(const std::string resourcePath)
    {
        // Get base path from resource path
        auto baseIndex = resourcePath.find("base");
        if (baseIndex == -1)
        {
            ThrowError(1,
                "Failed to load .resource file.",
                "The .resource file must be located in your Doom Eternal \"base\" directory or its subdirectories."
            );
            return 0;
        }
        _BasePath = resourcePath.substr(0, baseIndex + 4);
        return 1;
    }
    void SAMUEL::LoadPackageMapSpec()
    {
        try
        {
            // read input filestream into stringstream
            std::ifstream inputStream = std::ifstream(_BasePath + (char)fs::path::preferred_separator + "packagemapspec.json");
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
        appendList = _PackageMapSpec.GetFilesByResourceName(_BasePath + (char)fs::path::preferred_separator + resourceFileName);

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
            std::string filePath = _BasePath + (char)fs::path::preferred_separator + *i;

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
    void SAMUEL::ExportAll(const std::string outputDirectory)
    {
        _Exporter.Init(_ResourceFile, _StreamDBFileData, outputDirectory);
        _Exporter.ExportTGAFiles(_StreamDBFileData);
        _Exporter.ExportMD6Files(_StreamDBFileData);
        _Exporter.ExportLWOFiles(_StreamDBFileData);
        return;
    }
    void SAMUEL::ExportSelected(const std::string outputDirectory, const std::vector<std::vector<std::string>> userSelectedFileList)
    {
        _Exporter.InitFromList(_ResourceFile, _StreamDBFileData, outputDirectory, userSelectedFileList);
        _Exporter.ExportTGAFiles(_StreamDBFileData);
        _Exporter.ExportMD6Files(_StreamDBFileData);
        _Exporter.ExportLWOFiles(_StreamDBFileData);
        return;
    }
    bool SAMUEL::CheckDependencies()
    {
        if (!FindOodleDLL())
            return 0;
        return 1;
    }
    bool SAMUEL::Init(const std::string resourcePath)
    {     
        if (!FindBasePath(resourcePath))
            return 0;

        LoadPackageMapSpec();
        return 1;
    }
}
