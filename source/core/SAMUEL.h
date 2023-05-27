#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "idFileTypes/PackageMapSpec.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "Common.h"
#include "ExportManager.h"
#include "Oodle.h"
#include "Utilities.h"
#include "ResourceManager.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    class SAMUEL {
    public:

// Startup and resource loader functions
        bool Init(const std::string &resourcePath);

        bool LoadResource(const std::string &fileName);

        // File export functions
        bool ExportFiles(const fs::path &outputDirectory, const std::vector<std::vector<std::string>> &filesToExport);

        [[nodiscard]] bool hasResourceLoadError() const { return m_hasResourceLoadError; }

        const std::string &GetLastErrorMessage() { return m_lastErrorMessage; }

        const std::string &GetLastErrorDetail() { return m_lastErrorDetail; }

        ResourceManager &resourceManager() { return m_resourceManager; }

    private:
        bool m_hasFatalError = false;
        bool m_hasResourceLoadError = false;
        std::string m_lastErrorMessage;
        std::string m_lastErrorDetail;
        std::string m_basePath;
        std::string m_resourcePath;
        std::string m_resourceFileName;
        ResourceManager m_resourceManager;

        // Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
        void ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail = "");

        // Called on startup
        bool SetBasePath(const std::string &resourcePath);

        // Loads all global *.resources (needed for LWO export)
        void LoadGlobalResources();
    };
}