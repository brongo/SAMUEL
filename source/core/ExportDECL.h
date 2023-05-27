#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "idFileTypes/ResourceFile.h"

#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    class DECLExportTask
    {
        public:

        DECLExportTask(const ResourceManager &resourceManager, const std::string &resourceName);
        bool exportDECL(const fs::path &exportPath);

        const std::string &fileName() { return m_fileName; }
        const std::string &resourcePath() { return m_resourcePath; }

    private:

        const ResourceManager &m_resourceManager;
        std::string m_fileName;
        std::string m_resourcePath;
    };
}