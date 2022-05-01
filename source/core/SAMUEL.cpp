#include "SAMUEL.h"

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN
{
    // Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
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

    // Sets _BasePath based on given resourcePath
    bool SAMUEL::SetBasePath(const std::string resourcePath)
    {
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

    // Read the game packagemapspec.json file into memory
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
            ThrowError(1,"Failed to load packagemapspec.json.");
            return;
        }
    }

    // Populate _StreamDBFileList based on currently loaded *.resources
    void SAMUEL::UpdateStreamDBFileList(const std::string resourceFileName)
    {
        std::vector<std::string> appendList;

        if (resourceFileName == "gameresources.resources")
            appendList = _PackageMapSpec.GetFilesByResourceName(_BasePath + (char)fs::path::preferred_separator + resourceFileName);
        else
            appendList = _PackageMapSpec.GetFilesByResourceName(resourceFileName);

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

        // Add EternalMod.streamdb file if needed (used for custom model mods)
        fs::path customStreamDBPath = fs::path(_BasePath) / "EternalMod.streamdb";

        if (fs::exists(customStreamDBPath) && _StreamDBFileList[0] != "EternalMod.streamdb")
            _StreamDBFileList.insert(std::begin(_StreamDBFileList), "EternalMod.streamdb");
    
        return;
    }
    
    // Read all *.streamdb file entries from _StreamDBFileList into memory
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

    // Loads all global *.resources data into memory (needed for LWO export)
    void SAMUEL::LoadGlobalResources()
    {
        // List of globally loaded *.resources, in order of load priority
        std::vector<std::string> globalResourceList =
        {
            "gameresources_patch1.resources",
            "warehouse_patch1.resources",
            "gameresources_patch2.resources",
            "gameresources.resources",
            "warehouse.resources"
        };

        // Load globals
        for (int i = 0; i < globalResourceList.size(); i++)
        {
            RESOURCES_ARCHIVE globalResource;
            globalResource.ResourceName = globalResourceList[i];
            globalResource.ResourcePath = fs::path(_BasePath) / fs::path(globalResource.ResourceName);
            globalResource.ResourcePath.make_preferred();

            ResourceFileReader reader(globalResource.ResourcePath.string());
            globalResource.Entries = reader.ParseResourceFile();
            _GlobalResources->Files.push_back(globalResource);
        }
        
        // If current *.resources file is a global resource, we can stop here
        if ((_ResourceFileName.find("gameresources") != -1) || (_ResourceFileName.find("warehouse") != -1))
            return;

        // If current *.resources file is NOT a patch, we can stop here
        if (_ResourceFileName.rfind("_patch") == -1)
            return;

        // Otherwise, we need to load the non-patch version of this *.resources file alongside our globals
        size_t patchSeparator = _ResourceFileName.rfind("_patch");
        std::string unpatchedResourceName = _ResourceFileName.substr(0, patchSeparator);
        fs::path unpatchedResourcePath = fs::path(_ResourcePath).remove_filename() / fs::path(unpatchedResourceName).replace_extension(".resources");
        unpatchedResourcePath.make_preferred();

        RESOURCES_ARCHIVE unpatchedResource;
        unpatchedResource.ResourcePath = unpatchedResourcePath;
        unpatchedResource.ResourceName = unpatchedResourcePath.filename().string();

        ResourceFileReader reader(unpatchedResource.ResourcePath.string());
        unpatchedResource.Entries = reader.ParseResourceFile();
        _GlobalResources->Files.push_back(unpatchedResource);

        return;
    }

    // Main resource loading function
    bool SAMUEL::LoadResource(const std::string resourcePath)
    {
        _HasResourceLoadError = 0;
        _ResourcePath = resourcePath;
        _ResourceFileName = fs::path(_ResourcePath).filename().string();

        // Clear any existing .resources data
        _ResourceData.clear();
        _GlobalResources->Files.clear();

        // Make sure this is a *.resources file
        if (_ResourcePath.rfind(".resources") == -1)
        {
            ThrowError(0, "Not a valid .resources file.", "Please load a file with the .resources or .resources.backup file extension.");
            _HasResourceLoadError = 1;
            return 0;
        }

        // Load the currently requested *.resources file + globals.
        try
        {
            ResourceFileReader reader(_ResourcePath);
            _ResourceData = reader.ParseResourceFile();
            LoadGlobalResources();
        }
        catch (...)
        {
            ThrowError(0, "Failed to read .resources file.", "Please load a file with the .resources or .resources.backup file extension.");
            _HasResourceLoadError = 1;
            return 0;
        }

        // Load .streamdb data
        try
        {
            UpdateStreamDBFileList("gameresources.resources");      
            UpdateStreamDBFileList("warehouse.resources");          
            UpdateStreamDBFileList(_ResourcePath);   
            ReadStreamDBFiles();
        }
        catch (...)
        {
            ThrowError(1, "Failed to read .streamdb file data.");
            return 0;
        }

        return 1;
    }
    
    // Main file export function
    bool SAMUEL::ExportFiles(const fs::path outputDirectory, const std::vector<std::vector<std::string>> filesToExport)
    {
        ExportManager exportManager;
        return exportManager.ExportFiles(_GlobalResources, _ResourceData, _ResourcePath, _StreamDBFileData, outputDirectory, filesToExport);
    }

    bool SAMUEL::Init(const std::string resourcePath, GLOBAL_RESOURCES& globalResources)
    {     
        _GlobalResources = &globalResources;
        if (!SetBasePath(resourcePath))
            return 0;

        if (!oodleInit(_BasePath))
        {
            ThrowError(1,
                "Failed to load the oodle dll.",
                "Make sure the oo2core_8_win64.dll file is present in your game directory."
            );
            return 0;
        }

        LoadPackageMapSpec();
        return 1;
    }
}