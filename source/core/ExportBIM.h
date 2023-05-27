#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "exportTypes/DDSHeader.h"
//#include "exportTypes/PNG.h"

#include "idFileTypes/BIM.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "ExportManager.h"
#include "Oodle.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    class BIMExportTask {
    public:

        BIMExportTask(const ResourceManager &resourceManager, const std::string &resourceName);

        bool exportBIMImage(const fs::path &exportPath, bool reconstructZ = false);

        const std::string &fileName() { return m_fileName; }
        const std::string &resourcePath() { return m_resourcePath; }

    private:

        const ResourceManager &m_resourceManager;
        std::string m_fileName;
        std::string m_resourcePath;

    };
}