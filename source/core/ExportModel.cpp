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
                                          const std::vector<MaterialInfo> &materials) {


        fs::path materialFile;
        fs::path outputFile;

        CAST castFile;

        castFile.ConvertFromMD6(mesh, md6Skl, materials);

        return std::move(castFile);
    }
//
//    // Write the MD6Mesh material data to MTL file. File is written to <ModelExportPath>
//    void ModelExportTask::WriteMTLFile() {
//        std::ofstream mtlfile;
//        fs::path materialFile = ModelExportPath / fs::path(_FileName).filename().replace_extension(".mtl");
//        mtlfile.open(materialFile.string(), std::ios::out);
//
//        for (auto &mtlNum: MaterialData) {
//            fs::path mtlPath = mtlNum.DeclFileName;
//            std::string mtlName = mtlPath.filename().replace_extension("").string();
//
//            fs::path diffuseTexture;
//            fs::path specularTexture;
//            fs::path normalTexture;
//
//            for (int j = 0; j < mtlNum.TextureTypes.size(); j++) {
//                // diffuse types
//                if (mtlNum.TextureTypes[j] == "albedo")
//                    diffuseTexture = mtlNum.TextureNames[j];
//                if (mtlNum.TextureTypes[j] == "eyealbedomap")
//                    diffuseTexture = mtlNum.TextureNames[j];
//
//                // specular types
//                if (mtlNum.TextureTypes[j] == "specular")
//                    specularTexture = mtlNum.TextureNames[j];
//
//                // normal types
//                if (mtlNum.TextureTypes[j] == "normal")
//                    normalTexture = mtlNum.TextureNames[j];
//                if (mtlNum.TextureTypes[j] == "eyeemissivemap")
//                    normalTexture = mtlNum.TextureNames[j];
//            }
//
//            if (!diffuseTexture.empty())
//                diffuseTexture = "images" / diffuseTexture.filename().replace_extension(".png");
//
//            if (!specularTexture.empty())
//                specularTexture = "images" / specularTexture.filename().replace_extension(".png");
//
//            if (!normalTexture.empty())
//                normalTexture = "images" / normalTexture.filename().replace_extension(".png");
//
//            mtlfile << "newmtl " + mtlName + " \n";
//            mtlfile
//                    << "illum 4 \n";                                        // DOOM Eternal uses Maya, which always exports as illum 4.
//            mtlfile << "Kd 0.00 0.00 0.00 \n";
//            mtlfile << "Ka 0.00 0.00 0.00 \n";
//            mtlfile << "Ks 0.50 0.50 0.50 \n";
//
//            if (!diffuseTexture.empty())
//                mtlfile << "map_Kd " + diffuseTexture.string() + " \n";
//
//            if (!normalTexture.empty())
//                mtlfile << "map_bump " + normalTexture.string() + " \n";
//
//            if (!specularTexture.empty())
//                mtlfile << "map_Ks " + specularTexture.string() + " \n";
//        }
//
//        mtlfile.close();
//    }
//
//    // Export Material2 DECL files used by this model. m_files are written to <ModelExportPath>/material2/
//    void ModelExportTask::ExportMaterial2Decls(const std::vector<ResourceEntry> &resourceData) {
//        for (auto &material: MaterialData) {
//            bool found = false;
//            std::string targetFileName = "generated/decls/material2/" + material.DeclFileName + ".decl";
//
//            // Construct .decl export path
//            fs::path targetFilePath(targetFileName);
//            fs::path outputFile = ModelExportPath / "material2" / targetFilePath.filename();
//
//            // Locate this file in current .resources
//            for (const auto &file: resourceData) {
//                if (file.Version != 0)
//                    continue;
//
//                if (file.Name == targetFileName) {
//                    DECLExportTask declExportTask(file);
//                    found = declExportTask.Export(outputFile, ResourcePath);
//                    break;
//                }
//            }
//
//            if (found)
//                continue;
//
////            // Unable to locate file in current .resources, search all global archives
////            for (const auto &File: globalResources->Files) {
////                for (int k = 0; k < File.Entries.size(); k++) {
////                    if (File.Entries[k].Version != 0)
////                        continue;
////
////                    if (File.Entries[k].Name == targetFileName) {
////                        DECLExportTask declExportTask(File.Entries[k]);
////                        found = declExportTask.Export(outputFile, File.ResourcePath.string());
////                        break;
////                    }
////                }
////                if (found)
////                    break;
////            }
//        }
//    }
//
//    // Parse Material2 DECL files used by this MD6Mesh model, to determine which BIM textures need to be exported.
//    void ModelExportTask::ReadMaterial2Decls() {
//        for (auto &material: MaterialData) {
//            // model path
//            fs::path materialFolder = ModelExportPath / "material2";
//
//            // decl path
//            fs::path declFileName = material.DeclFileName + ".decl";
//            fs::path materialFile = materialFolder / declFileName.filename();
//
//            std::ifstream inputStream = std::ifstream(materialFile);
//            if (inputStream.is_open()) {
//                DeclFile declFile;
//                declFile.SetFileName(materialFile.string());
//                std::string line;
//                while (std::getline(inputStream, line)) {
//                    DeclSingleLine declSingleLine;
//                    declSingleLine.ReadFromStream(inputStream, line);
//                    declFile.SetLineData(declSingleLine);
//                    declFile.LineCount++;
//                }
//
//                material.ParsedDeclFile = declFile;
//                inputStream.close();
//            }
//        }
//
//        // find textures referenced in this file and add to MaterialData
//        for (auto &material: MaterialData) {
//            DeclFile &thisDeclFile = material.ParsedDeclFile;
//            for (int lineNumber = 0; lineNumber < thisDeclFile.LineCount; lineNumber++) {
//                DeclSingleLine lineData = thisDeclFile.GetLineData(lineNumber);
//                if (lineData.GetLineVariable() != "filePath")
//                    continue;
//
//                DeclSingleLine prevLineData = thisDeclFile.GetLineData(lineNumber - 1);
//                std::string textureType = prevLineData.GetLineVariable();
//                std::string textureName = stripQuotes(lineData.GetLineValue());
//
//                if (textureName != "_default") // these don't matter
//                {
//                    material.TextureNames.push_back(textureName);
//                    material.TextureTypes.push_back(textureType);
//                }
//            }
//        }
//    }
//
//    // Export BIM textures used by this MD6Mesh model. m_files are written to <ModelExportPath>/images/
//    void ModelExportTask::ExportBIMTextures(const std::vector<ResourceEntry> &resourceData,
//                                            const MaterialInfo &materialInfo) const {
//        for (uint64_t i = 0; i < materialInfo.TextureNames.size(); i++) {
//            bool found = false;
//            std::string targetFileName = materialInfo.TextureNames[i];
//
//            // Construct .bim export path
//            fs::path targetFilePath(targetFileName);
//            fs::path outputFile = ModelExportPath / "images" / targetFilePath.filename().replace_extension(".png");
//
//            // Locate this file in .resources
//            for (const auto &file: resourceData) {
//                // Skip non-image files
//                if (file.Version != 21)
//                    continue;
//
//                // Drop $ qualifiers from resourceData.m_name
//                std::string compName = file.Name;
//                size_t pos = compName.find('$');
//
//                if (pos != -1)
//                    compName = compName.substr(0, pos);
//
//                // Found
//                if (compName == targetFileName) {
//                    // Check for smoothness texture - will have a slash somewhere in the truncated $ stuff
//                    if (pos != -1) {
//                        std::string smoothnessCheck = file.Name.substr(pos, file.Name.length());
//                        if ((smoothnessCheck.rfind('/') != -1) && (materialInfo.TextureTypes[i] != "smoothness"))
//                            continue;
//                    }
//
//                    BIMExportTask bimExportTask(file);
//                    found = bimExportTask.Export(outputFile, ResourcePath, true);
//                    break;
//                }
//            }
//
//            if (found)
//                continue;
//
//            // Unable to locate file in current .resources, search all global archives
////            for (const auto &file: globalResources->Files) {
////                // Search each global archive
////                for (const ResourceEntry &entry: file.Entries) {
////                    // Skip non-image files
////                    if (entry.Version != 21)
////                        continue;
////
////                    // Drop $ qualifiers from resourceData.m_name
////                    std::string compName = entry.Name;
////                    size_t pos = compName.find('$');
////
////                    if (pos != -1)
////                        compName = compName.substr(0, pos);
////
////                    // Found
////                    if (compName == targetFileName) {
////                        // Check for smoothness texture - will have a slash somewhere in the truncated $ stuff
////                        if (pos != -1) {
////                            std::string smoothnessCheck = entry.Name.substr(pos,
////                                                                            entry.Name.length());
////                            if ((smoothnessCheck.rfind('/') != -1) && (materialInfo.TextureTypes[i] != "smoothness"))
////                                continue;
////                        }
////
////                        BIMExportTask bimExportTask(entry);
////                        found = bimExportTask.Export(outputFile, file.ResourcePath.string(),
////                                                     streamDBFiles, true);
////                        break;
////                    }
////                }
////
////                if (found)
////                    break;
////        }
//        }
//    }

// Main export function for models.
// Return 1 for success, 0 for failure.
    bool ModelExportTask::exportMD6Model(const fs::path &exportPath) {
        MD6Mesh mesh(m_resourceManager, m_resourcePath);
        if (!mesh.loaded())
            return false;

        fs::path outputFile = exportPath / fs::path(m_fileName).replace_extension(".cast");



//        // Serialize model data and get materials (LWO)
//        if (modelType == 67) {
//            _LWO.Serialize(_LWOHeader, modelData);
//            for (auto &meshInfo: _LWOHeader.m_meshInfo) {
//                MaterialInfo materialInfo;
//                materialInfo.DeclFileName = meshInfo.MaterialDeclName;
//                MaterialData.push_back(materialInfo);
//            }
//        }

        for (auto &meshInfo: mesh.header().m_meshInfo) {
            MaterialInfo materialInfo;
            materialInfo.DeclFileName = meshInfo.MaterialDeclName;
            m_materialData.push_back(materialInfo);
        }
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
        for (auto &item: m_materialData) {
            std::string materialFilename = "generated/decls/material2/" + item.DeclFileName + ".decl";
            DECLExportTask task(m_resourceManager, materialFilename);

            const std::filesystem::path &declOutputPath =
                    outputFile.parent_path() / "materials";
            if (!createPaths(declOutputPath)) {
                return false;
            }
            task.exportDECL(declOutputPath);
            DeclFile declFile(m_resourceManager, materialFilename);
            declFile.parse();
            const jsonxx::Object &materialData = declFile.json();
            const jsonxx::Object &editData = materialData.get<jsonxx::Object>("edit");
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
        for (const auto &material: m_materialData) {
            for (const auto &[type, textureName]: material.TextureMapping) {
                BIMExportTask task(m_resourceManager, textureName);
                const std::filesystem::path &bimOutputPath =
                        outputFile.parent_path() / "textures";
                createPaths(bimOutputPath);
                task.exportBIMImage(bimOutputPath, type == "normal");
            }
        }


        MD6Skl skl(m_resourceManager, mesh.header().m_sklFilename);
        CAST castFile = castMeshFromMD6(mesh, skl, m_materialData);
        // Create output directories if needed
        if (!createPaths(outputFile.parent_path())) {
            return false;
        }

        std::ofstream file;
        file.open(outputFile.string(), std::ios::out | std::ios::binary);

        castFile.toFile(file);

//        WriteMTLFile();
        return true;
    }

}
