#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "exportTypes/DDSHeader.h"

#include "idFileTypes/BIM.h"
#include "idFileTypes/DECL.h"
#include "idFileTypes/LWO.h"
#include "idFileTypes/MD6Mesh.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "Common.h"
#include "ExportDECL.h"
#include "ExportBIM.h"
#include "Oodle.h"
#include "Utilities.h"
#include "exportTypes/CAST.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    // Holds material2 .decl data used by the model

    struct MaterialInfo {
        std::string DeclFileName;
        std::map<std::string, std::string> TextureMapping;
    };

    class ModelExportTask {
    public:


        ModelExportTask(const ResourceManager &resourceManager, const std::string &resourceName);

        CAST castMeshFromMD6(const MD6Mesh &mesh, const MD6Skl &md6Skl, const std::vector<MaterialInfo> &materials) const;
        CAST castMeshFromLWO(const LWO &mesh, const std::vector<MaterialInfo> &materials) const;

        bool exportMD6Model(const fs::path &exportPath);

        bool exportLWOModel(const fs::path &exportPath);

        bool exportMaterialsAndTextures(const fs::path& exportPath);

    private:

        // m_name of the file we are exporting, as it appears in a *.resources file
        const ResourceManager &m_resourceManager;
        std::string m_fileName;
        std::string m_resourcePath;
        fs::path m_modelExportPath;
        std::vector<MaterialInfo> m_materialData;


    };
}