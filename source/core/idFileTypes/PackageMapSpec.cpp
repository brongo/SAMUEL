#include "PackageMapSpec.h"

namespace HAYDEN {
    // Construct from .json
    PackageMapSpec::PackageMapSpec(const std::string &json) {
        jsonxx::Object packageMapSpecJson;
        packageMapSpecJson.parse(json);

        PackageMapSpecFile packageMapSpecFile;
        jsonxx::Array files = packageMapSpecJson.get<jsonxx::Array>("files");
        m_files.reserve(files.size());

        for (int32_t i = 0; i < files.size(); i++) {
            jsonxx::Object file = files.get<jsonxx::Object>(i);
            packageMapSpecFile.m_name = file.get<jsonxx::String>("name");
            m_files.push_back(packageMapSpecFile);
        }

        PackageMapSpecMapFileRef packageMapSpecMapFileRef;
        jsonxx::Array mapFileRefs = packageMapSpecJson.get<jsonxx::Array>("mapFileRefs");
        m_mapFileRefs.reserve(mapFileRefs.size());

        for (int32_t i = 0; i < mapFileRefs.size(); i++) {
            jsonxx::Object mapFileRef = mapFileRefs.get<jsonxx::Object>(i);
            packageMapSpecMapFileRef.m_file = (int32_t) mapFileRef.get<jsonxx::Number>("file");
            packageMapSpecMapFileRef.m_map = (int32_t) mapFileRef.get<jsonxx::Number>("map");
            m_mapFileRefs.push_back(packageMapSpecMapFileRef);
        }

        PackageMapSpecMap packageMapSpecMap;
        jsonxx::Array maps = packageMapSpecJson.get<jsonxx::Array>("maps");
        m_maps.reserve(maps.size());

        for (int32_t i = 0; i < maps.size(); i++) {
            jsonxx::Object map = maps.get<jsonxx::Object>(i);
            packageMapSpecMap.m_name = map.get<jsonxx::String>("name");
            m_maps.push_back(packageMapSpecMap);
        }
        m_loaded = true;
    }

    // dump all PackageMapSpec data
    std::string PackageMapSpec::dump() const {
        jsonxx::Object packageMapSpecJson;
        jsonxx::Array files;
        jsonxx::Array mapFileRefs;
        jsonxx::Array maps;

        for (auto &file: m_files) {
            jsonxx::Object jsonFile;
            jsonFile << "name" << file.name();
            files << jsonFile;
        }

        for (auto &mapFileRef: m_mapFileRefs) {
            jsonxx::Object jsonMapFileRef;
            jsonMapFileRef << "file" << mapFileRef.file();
            jsonMapFileRef << "map" << mapFileRef.map();
            mapFileRefs << jsonMapFileRef;
        }

        for (auto &map: m_maps) {
            jsonxx::Object jsonMap;
            jsonMap << "name" << map.m_name;
            maps << jsonMap;
        }

        packageMapSpecJson << "files" << files;
        packageMapSpecJson << "mapFileRefs" << mapFileRefs;
        packageMapSpecJson << "maps" << maps;

        return std::move(packageMapSpecJson.json());
    }

    // Convert path separators to "/", to match packageMapSpec format
    std::string PackageMapSpec::normalizedFilePath(const std::string &filePath) {
        std::string output = filePath;
        std::replace(output.begin(), output.end(), '\\', '/');
        return std::move(output);
    }

    // Confirm filepath contains "Base" directory
    bool PackageMapSpec::inBaseDirectory(const std::string &filePath) {
        size_t strPos = filePath.rfind("base");
        if (strPos == -1)
            return false;
        return true;
    }

    // Returns filepath relative to "Base" directory
    std::string PackageMapSpec::getRelativeFilePath(const std::string &filePath) {
        size_t strPos = filePath.rfind("base");
        std::string relativePath = filePath.substr(strPos + 5, filePath.length() - strPos);
        return std::move(relativePath);
    }

    // Returns file index for a given filename
    size_t PackageMapSpec::getFileIndexByFileName(const std::string &filePath) const {
        auto fileIterator = std::find_if(m_files.begin(), m_files.end(),
                                         [&](const PackageMapSpecFile &file) { return file.name() == filePath; });
        if (fileIterator == m_files.end())
            return -1;
        return std::distance(m_files.begin(), fileIterator);
    }

    // Returns map index for a given file index
    size_t PackageMapSpec::getMapIndexByFileIndex(size_t fileIndex) const {
        auto mapFileRefIterator = std::find_if(m_mapFileRefs.begin(), m_mapFileRefs.end(),
                                               [&](const PackageMapSpecMapFileRef &mapFileRef) {
                                                   return mapFileRef.file() == fileIndex;
                                               });

        if (mapFileRefIterator == m_mapFileRefs.end())
            return -1;

        return m_mapFileRefs[std::distance(m_mapFileRefs.begin(), mapFileRefIterator)].map();
    }

    // Returns list of .resources and .streamdbs loaded in a certain map
    std::vector<std::string> PackageMapSpec::getFilesByResourceName(const std::string &resourceFileName) const {
        std::vector<std::string> fileList;

        std::string normalizedPath = normalizedFilePath(resourceFileName);

        if (!inBaseDirectory(normalizedPath))
            return std::move(fileList);

        if (normalizedPath.rfind(".backup") != -1)
            normalizedPath = normalizedPath.substr(0, normalizedPath.rfind(".backup"));

        normalizedPath = getRelativeFilePath(normalizedPath);
        size_t fileIndex = getFileIndexByFileName(normalizedPath);
        size_t mapIndex = getMapIndexByFileIndex(fileIndex);

        if (mapIndex == -1 || fileIndex == -1)
            return std::move(fileList);

        for (const auto &mapFileRef: m_mapFileRefs) {
            if (mapFileRef.map() != mapIndex)
                continue;
            fileList.push_back(m_files[mapFileRef.file()].name());
        }
        return std::move(fileList);
    }

    void PackageMapSpec::reset() {
        m_mapFileRefs.clear();
        m_files.clear();
        m_maps.clear();
        m_loaded = false;
    }


}