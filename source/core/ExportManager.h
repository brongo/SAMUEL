#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "ResourceManager.h"
#include "Common.h"
#include "ExportBIM.h"
#include "ExportCOMP.h"
#include "ExportDECL.h"
#include "ExportModel.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    enum class ExportType {
        DECL = 0,
        COMP = 1,
        BIM = 21,
        MD6 = 31,
        LWO = 67
    };

    class ExportManager {
    public:
        static std::string getResourceFolder(const std::string &resourcePath);

        static fs::path buildOutputPath(const std::string &filePath, const fs::path &outputDirectory, ExportType exportType,
                                 const std::string &resourceFolder);

        bool exportFiles(const HAYDEN::ResourceManager &resourceManager,
                         const std::string &resourcePath,
                         const fs::path &outputDirectory,
                         const std::vector<std::vector<std::string>> &filesToExport);

    };
}