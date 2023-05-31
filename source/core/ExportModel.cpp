#include "ExportModel.h"
#include "source/core/exportTypes/CAST.h"

namespace HAYDEN {
    // Constructor
    ModelExportTask::ModelExportTask(const ResourceManager &resourceManager, const std::string &resourceName)
            : m_resourceManager(resourceManager) {
        m_resourcePath = resourceName;
        m_fileName = getFilename(resourceName);
    }

    // Write the model data to OBJ file
    CAST ModelExportTask::castMeshFromMD6(const MD6Mesh &mesh, const MD6Skl &md6Skl,
                                          const std::vector<MaterialInfo> &materials) const {
        CAST castFile;

        castFile.ConvertFromMD6(mesh, md6Skl, materials);

        return std::move(castFile);
    }

    CAST ModelExportTask::castMeshFromLWO(const LWO &mesh, const std::vector<MaterialInfo> &materials) const {
        CAST castFile;

        castFile.ConvertFromLWO(mesh, materials);

        return std::move(castFile);
    }

// Main export function for models.
// Return 1 for success, 0 for failure.
    bool ModelExportTask::exportMD6Model(const fs::path &exportPath) {
        std::cout << "Exporing \"" << m_resourcePath << "\" to " << exportPath << std::endl;
        MD6Mesh mesh(m_resourceManager, m_resourcePath);
        if (!mesh.loaded())
            return false;

        fs::path outputFile = exportPath / fs::path(m_fileName).replace_extension(".cast");

        for (auto &meshInfo: mesh.header().m_meshInfo) {
            MaterialInfo materialInfo;
            materialInfo.DeclFileName = meshInfo.MaterialDeclName;
            m_materialData.push_back(materialInfo);
        }

        if (!exportMaterialsAndTextures(outputFile.parent_path())) {
            return false;
        }

        MD6Skl skl(m_resourceManager, mesh.header().m_sklFilename);
        CAST castFile = castMeshFromMD6(mesh, skl, m_materialData);
        // Create output directories if needed
        if (!createPaths(outputFile.parent_path())) {
            return false;
        }

        std::ofstream file;
        file.open(outputFile.string(), std::ios::out | std::ios::binary);
        std::cout << "Writing CAST file to" << outputFile << std::endl;
        castFile.toFile(file);

        return true;
    }

    bool ModelExportTask::exportLWOModel(const fs::path &exportPath) {
        std::cout << "Exporing \"" << m_resourcePath << "\" to " << exportPath << std::endl;
        LWO lwoMesh(m_resourceManager, m_resourcePath);
        if (!lwoMesh.loaded())
            return false;

        fs::path outputFile = exportPath / fs::path(m_fileName).replace_extension(".cast");

        for (auto &meshInfo: lwoMesh.header().m_meshInfo) {
            MaterialInfo materialInfo;
            materialInfo.DeclFileName = meshInfo.MaterialDeclName;
            m_materialData.push_back(materialInfo);
        }
        if (!exportMaterialsAndTextures(outputFile.parent_path())) {
            return false;
        }

        CAST castFile = castMeshFromLWO(lwoMesh, m_materialData);
        // Create output directories if needed
        if (!createPaths(outputFile.parent_path())) {
            return false;
        }

        std::ofstream file;
        file.open(outputFile.string(), std::ios::out | std::ios::binary);
        std::cout << "Writing CAST file to" << outputFile << std::endl;
        castFile.toFile(file);
        return true;
    }

    bool ModelExportTask::exportMaterialsAndTextures(const fs::path &exportPath) {
// Remove any duplicate materials
        std::sort(m_materialData.begin(), m_materialData.end(), [](const MaterialInfo &a, const MaterialInfo &b) {
            return (a.DeclFileName < b.DeclFileName);
        });

        auto it = std::unique(m_materialData.begin(), m_materialData.end(),
                              [](const MaterialInfo &a, const MaterialInfo &b) {
                                  if (a.DeclFileName == b.DeclFileName)
                                      return 1;
                                  return 0;
                              });

        size_t listSize = it - m_materialData.begin();
        m_materialData.resize(listSize);
        //Gather textures and export materials
        for (auto &item: m_materialData) {
            std::string materialFilename = "generated/decls/material2/" + item.DeclFileName + ".decl";
            DECLExportTask task(m_resourceManager, materialFilename);

            const std::filesystem::path &declOutputPath = exportPath / "materials";
            if (!createPaths(declOutputPath)) {
                return false;
            }
            task.exportDECL(declOutputPath);
            DeclFile declFile(m_resourceManager, materialFilename);
            declFile.parse();
            const jsonxx::Object &materialData = declFile.json();
            const jsonxx::Object &editData = materialData.get<jsonxx::Object>("edit");
            if(!editData.has<jsonxx::Array>("RenderLayers")){
                continue;
            }
            const jsonxx::Array &renderLayersData = editData.get<jsonxx::Array>("RenderLayers");
            if (!renderLayersData.empty()) {
                const jsonxx::Object &textureParms = renderLayersData.get<jsonxx::Object>(0).get<jsonxx::Object>(
                        "parms");
                for (auto [key, value]: textureParms.kv_map()) {
                    const std::string &texturePath = value->get<jsonxx::Object>().get<jsonxx::String>("filePath");
                    item.TextureMapping.insert_or_assign(key, texturePath);
                }
            }
        }
        //Export texture
        for (const auto &material: m_materialData) {
            for (const auto &[type, textureName]: material.TextureMapping) {
                BIMExportTask task(m_resourceManager, textureName);
                const std::filesystem::path &bimOutputPath = exportPath / "textures";
                createPaths(bimOutputPath);
//                task.exportBIMImage(bimOutputPath, type == "normal");
            }
        }
        return true;
    }


}
