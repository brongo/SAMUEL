#include "../core/ResourceManager.h"
#include "../core/Oodle.h"
#include "../core/idFileTypes/MD6Mesh.h"
#include "source/core/ExportModel.h"
#include "source/core/ExportBIM.h"
#include "source/core/ExportManager.h"

int main(int argc, char **argv) {
    HAYDEN::oodleInit(R"(D:\SteamLibrary\steamapps\common\DOOMEternal\base)");
    HAYDEN::ResourceManager manager;
    bool res = manager.mountResourceFile(
            R"(D:\SteamLibrary\steamapps\common\DOOMEternal\base\gameresources.resources)");
    if (!res) {
        std::cout << "Failed to mount resource" << std::endl;
        return -1;
    }
    const char *source = "md6/characters/monsters/baron/base/assets/mesh/baron.md6mesh";
    const std::filesystem::path &outputDirectory = absolute(fs::path("./exports"));
    fs::path exportPath = HAYDEN::ExportManager::buildOutputPath(fs::path(source).stem().string(), outputDirectory,
                                                                 HAYDEN::ExportType::MD6, fs::path(source).parent_path().string());

    HAYDEN::ModelExportTask task(manager, source);
    task.exportMD6Model(outputDirectory / exportPath);

//
//    source = "models/monsters/baron/baron_cs_n.tga$streamed$mtlkind=normal";
//    HAYDEN::BIMExportTask task2(manager, source);
//    task2.exportBIMImage(outputDirectory / exportPath,true);
//    source = "generated/decls/material2/models/monsters/baron/baron_eyes.decl";
//    exportPath = HAYDEN::ExportManager::buildOutputPath(fs::path(source).stem().string(), outputDirectory,
//                                                                 HAYDEN::ExportType::DECL, fs::path(source).parent_path().string());
//
//    HAYDEN::DECLExportTask decl(manager,source);
//    decl.exportDECL(exportPath);

    return 0;
}