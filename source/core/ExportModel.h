#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "exportTypes/DDSHeader.h"
#include "exportTypes/OBJ.h"
#include "exportTypes/PNG.h"

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
#include "ResourceFileReader.h"
#include "Utilities.h"
#include "exportTypes/CAST.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    // Holds material2 .decl data used by the model
    struct MaterialInfo {
        DeclFile ParsedDeclFile;
        std::string DeclFileName;
        std::vector<std::string> TextureNames;
        std::vector<std::string> TextureTypes;
    };

    class ModelExportTask {
    public:


        explicit ModelExportTask(const ResourceManager &resourceManager, const std::string &resourceName);

        // OBJ export functions. Consider moving to OBJ.h
//        void WriteMTLFile();
//
        CAST castMeshFromMD6(const MD6Mesh &mesh, const MD6Skl &md6Skl);
//
//        // Dependency export functions (material2 .decls and BIM textures)
//        void ExportBIMTextures(const std::vector<ResourceEntry> &resourceData, const MaterialInfo &materialInfo) const;
//
//        void
//        ExportMaterial2Decls(const std::vector<ResourceEntry> &resourceData);
//
//        void ReadMaterial2Decls();

        bool exportMD6Model(const fs::path &exportPath);

    private:

        // m_name of the file we are exporting, as it appears in a *.resources file
        const ResourceManager &m_resourceManager;
        std::string m_fileName;
        std::string m_resourcePath;
        fs::path m_modelExportPath;
        std::vector<MaterialInfo> m_materialData;


    };
}