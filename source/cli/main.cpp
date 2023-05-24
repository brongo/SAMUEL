#include "../core/ResourceManager.h"
#include "../core/Oodle.h"
#include "../core/idFileTypes/MD6Mesh.h"
#include "source/core/ExportModel.h"
#include "source/core/ExportManager.h"

int main(int argc, char **argv) {
    HAYDEN::oodleInit(R"(D:\SteamLibrary\steamapps\common\DOOMEternal\base)");
    HAYDEN::ResourceManager manager;
    bool res = manager.mountResourceFile(
            R"(D:\SteamLibrary\steamapps\common\DOOMEternal\base\gameresources.resources)");
    if (!res) {
        std::cout << "Failed to mount resourceHeader" << std::endl;
    }
    const char *source = "md6/characters/monsters/baron/base/assets/mesh/baron.md6mesh";
    const std::filesystem::path &outputDirectory = absolute(fs::path("./exports"));
    fs::path exportPath = HAYDEN::ExportManager::buildOutputPath(fs::path(source).stem().string(), outputDirectory,
                                                                 HAYDEN::ExportType::MD6, source);

    HAYDEN::ModelExportTask task(manager, source);
    task.exportMD6Model(outputDirectory / exportPath);
    return 0;
}