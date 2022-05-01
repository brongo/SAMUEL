#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "idFileTypes/PackageMapSpec.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "ExportManager.h"

#include "Oodle.h"
#include "ResourceFileReader.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    class SAMUEL
    {
	public:

	    // Startup and resource loader functions
	    bool Init(const std::string resourcePath, GLOBAL_RESOURCES& GlobalResources);
	    bool LoadResource(const std::string fileName);

	    // File export functions
	    bool ExportFiles(const fs::path outputDirectory, const std::vector<std::vector<std::string>> filesToExport);
	    bool HasResourceLoadError() { return _HasResourceLoadError; }
	    std::string GetLastErrorMessage() { return _LastErrorMessage; }
	    std::string GetLastErrorDetail() { return _LastErrorDetail; }
	    std::vector<ResourceEntry> GetResourceData() { return _ResourceData; }

	private:
	    bool _HasFatalError = 0;
	    bool _HasResourceLoadError = 0;
	    std::string _LastErrorMessage;
	    std::string _LastErrorDetail;
	    std::string _BasePath;
	    std::string _ResourcePath;
            std::string _ResourceFileName;
	    std::vector<std::string> _StreamDBFileList;
	    std::vector<StreamDBFile> _StreamDBFileData; 
	    std::vector<ResourceEntry> _ResourceData;
	    PackageMapSpec _PackageMapSpec;
            GLOBAL_RESOURCES* _GlobalResources;

	    // Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
	    void ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail = "");

	    // Called on startup
	    bool SetBasePath(const std::string resourcePath);
	    void LoadPackageMapSpec();

	    // Loads all global *.resources (needed for LWO export)
	    void LoadGlobalResources();

	    // Reads streamdb data associated with this resource file (per packageMapSpec).
	    void UpdateStreamDBFileList(const std::string resourceFileName);
	    void ReadStreamDBFiles();
    };
}