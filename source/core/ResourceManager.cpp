#include <optional>
#include "ResourceManager.h"

#include "Utilities.h"

bool HAYDEN::ResourceManager::mountResourceFile(const fs::path &path) {
    ResourceFile resourceFile(path);
    if (resourceFile.loaded()) {
        bool failedToMountStreamDB = false;
        if (!m_mapSpec.loaded() && loadPackageMapSpec(path.parent_path() / "packagemapspec.json")) {
            const std::vector<std::string> toMount = m_mapSpec.getFilesByResourceName(path);
            std::for_each(toMount.begin(), toMount.end(),
                          [&resourceFile, &path, &failedToMountStreamDB](const std::string &item) {
                              if (item.find(".streamdb") != -1) {
                                  failedToMountStreamDB |= !resourceFile.mountStreamDB(path.parent_path() / item);
                              }
                          });
        }
        m_resourceFiles.push_back(std::move(resourceFile));
        return m_mapSpec.loaded() & !failedToMountStreamDB;

    }
    std::cerr << "Error: Failed to load " << path << " !" << std::endl;
    return false;
}

std::optional<std::vector<uint8_t>> HAYDEN::ResourceManager::queryFileByName(const std::string &name) const {
    for (const auto &item: m_resourceFiles) {
        if (auto file = item.queryFileByName(name)) {
            return file;
        }
    }
    return std::nullopt;
}

std::optional<std::vector<uint8_t>> HAYDEN::ResourceManager::queryStreamDataByName(
        const std::string &name, uint64_t streamSize, int32_t mipCount) const {
    for (const auto &item: m_resourceFiles) {
        if (auto file = item.queryStreamDataByName(name, streamSize, mipCount)) {
            return file;
        }
    }
    return std::nullopt;
}

bool HAYDEN::ResourceManager::loadPackageMapSpec(const fs::path &path) {
    std::string jsonData = readFile(path);
    if (jsonData.empty()) {
        std::cerr << "Error: Failed to load PackageMapSpec.json!" << std::endl;
        return false;
    }
    m_mapSpec = PackageMapSpec(jsonData);
    return true;
}

void HAYDEN::ResourceManager::reset() {
    m_mapSpec.reset();
    m_resourceFiles.clear();
}

std::vector<std::pair<std::string, std::string>> HAYDEN::ResourceManager::filenameList() const {
    std::vector<std::pair<std::string, std::string>> allFiles;
    for (const auto &item: m_resourceFiles) {
        std::vector<std::pair<std::string, std::string>> files = item.filenameList();
        allFiles.insert(allFiles.end(), files.begin(), files.end());
    }
    return std::move(allFiles);
}

std::vector<HAYDEN::ResourceEntry> HAYDEN::ResourceManager::fileList() const {
    std::vector<HAYDEN::ResourceEntry> allFiles;
    for (const auto &item: m_resourceFiles) {
        std::vector<HAYDEN::ResourceEntry> files = item.fileList();
        allFiles.insert(allFiles.end(), files.begin(), files.end());
    }
    return std::move(allFiles);
}
