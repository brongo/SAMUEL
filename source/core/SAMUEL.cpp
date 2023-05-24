#include "SAMUEL.h"

#include <utility>

using namespace HAYDEN;
namespace fs = std::filesystem;

namespace HAYDEN {
    // Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
    void SAMUEL::ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail) {
        m_lastErrorMessage = std::move(errorMessage);
        m_lastErrorDetail = std::move(errorDetail);

        if (isFatal)
            m_hasFatalError = true;

        std::string consoleMsg = m_lastErrorMessage + " " + m_lastErrorDetail + "\n";
        fprintf(stderr, "%s", consoleMsg.c_str());
    }

    // Sets m_basePath based on given resourcePath
    bool SAMUEL::SetBasePath(const std::string &resourcePath) {
        auto baseIndex = resourcePath.find("base");
        if (baseIndex == -1) {
            ThrowError(true,
                       "Failed to load .resource file.",
                       "The .resource file must be located in your Doom Eternal \"base\" directory or its subdirectories."
            );
            return false;
        }
        m_basePath = resourcePath.substr(0, baseIndex + 4);
        return true;
    }

    // Loads all global *.resources data into memory (needed for LWO export)
    void SAMUEL::LoadGlobalResources() {
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
        for (const auto &i: globalResourceList) {
            m_resourceManager.mountResourceFile(fs::path(m_basePath) / i);
        }

        // If current *.resources file is a global resource, we can stop here
        if ((m_resourceFileName.find("gameresources") != -1) || (m_resourceFileName.find("warehouse") != -1))
            return;

        // If current *.resources file is NOT a patch, we can stop here
        if (m_resourceFileName.rfind("_patch") == -1)
            return;

        // Otherwise, we need to load the non-patch version of this *.resources file alongside our globals
        size_t patchSeparator = m_resourceFileName.rfind("_patch");
        std::string unpatchedResourceName = m_resourceFileName.substr(0, patchSeparator);
        fs::path unpatchedResourcePath = fs::path(m_resourcePath).remove_filename() /
                                         fs::path(unpatchedResourceName).replace_extension(".resources");
        unpatchedResourcePath.make_preferred();

        m_resourceManager.mountResourceFile(unpatchedResourcePath);
    }

    // Main resource loading function
    bool SAMUEL::LoadResource(const std::string &resourcePath) {
        m_hasResourceLoadError = false;
        m_resourcePath = resourcePath;
        m_resourceFileName = fs::path(m_resourcePath).filename().string();

        m_resourceManager.reset();

        // Make sure this is a *.resources file
        if (m_resourcePath.rfind(".resources") == -1) {
            ThrowError(false, "Not a valid .resources file.",
                       "Please load a file with the .resources or .resources.backup file extension.");
            m_hasResourceLoadError = true;
            return false;
        }

        // Load the currently requested *.resources file + globals.
        try {
            m_resourceManager.mountResourceFile(resourcePath);
            LoadGlobalResources();
        }
        catch (...) {
            ThrowError(false, "Failed to read .resources file.",
                       "Please load a file with the .resources or .resources.backup file extension.");
            m_hasResourceLoadError = true;
            return false;
        }
        return true;
    }

    // Main file export function
    bool
    SAMUEL::ExportFiles(const fs::path &outputDirectory, const std::vector<std::vector<std::string>> &filesToExport) {
        ExportManager exportManager;
        return exportManager.exportFiles(m_resourceManager, m_resourcePath, outputDirectory, filesToExport);
    }

    bool SAMUEL::Init(const std::string &resourcePath) {
        if (!SetBasePath(resourcePath))
            return false;

        if (!oodleInit(m_basePath)) {
            ThrowError(true,
                       "Failed to load the oodle dll.",
                       "Make sure the oo2core_8_win64.dll file is present in your game directory."
            );
            return false;
        }
        return true;
    }
}