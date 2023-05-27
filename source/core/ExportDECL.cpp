#include "ExportDECL.h"
#include "source/core/idFileTypes/DECL.h"

namespace HAYDEN {
    DECLExportTask::DECLExportTask(const ResourceManager &resourceManager, const std::string &resourceName)
            : m_resourceManager(resourceManager) {
        m_resourcePath = resourceName;
        m_fileName = getFilename(resourceName);
    }

    bool DECLExportTask::exportDECL(const fs::path &exportPath) {
        DeclFile decl(m_resourceManager, m_resourcePath);
        decl.parse();
        if (decl.data().empty())
            return false;
        fs::path outputFile = exportPath / fs::path(m_fileName).replace_extension(".decl");
        if (!writeToFilesystem(decl.data(), outputFile)) {
            return false;
        }
        if (!writeToFilesystem(decl.json().json(), outputFile.replace_extension(".json"))) {
            return false;
        }

        return true;
    }
}