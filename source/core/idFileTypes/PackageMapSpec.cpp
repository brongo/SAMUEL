#include "PackageMapSpec.h"

namespace HAYDEN
{
    // Construct from .json
    PackageMapSpec::PackageMapSpec(const std::string& json)
    {
        jsonxx::Object packageMapSpecJson;
        packageMapSpecJson.parse(json);

        PackageMapSpecFile packageMapSpecFile;
        jsonxx::Array files = packageMapSpecJson.get<jsonxx::Array>("files");
        Files.reserve(files.size());

        for (int32_t i = 0; i < files.size(); i++)
        {
            jsonxx::Object file = files.get<jsonxx::Object>(i);
            packageMapSpecFile.Name = file.get<jsonxx::String>("name");
            Files.push_back(packageMapSpecFile);
        }

        PackageMapSpecMapFileRef packageMapSpecMapFileRef;
        jsonxx::Array mapFileRefs = packageMapSpecJson.get<jsonxx::Array>("mapFileRefs");
        MapFileRefs.reserve(mapFileRefs.size());

        for (int32_t i = 0; i < mapFileRefs.size(); i++)
        {
            jsonxx::Object mapFileRef = mapFileRefs.get<jsonxx::Object>(i);
            packageMapSpecMapFileRef.File = mapFileRef.get<jsonxx::Number>("file");
            packageMapSpecMapFileRef.Map = mapFileRef.get<jsonxx::Number>("map");
            MapFileRefs.push_back(packageMapSpecMapFileRef);
        }

        PackageMapSpecMap packageMapSpecMap;
        jsonxx::Array maps = packageMapSpecJson.get<jsonxx::Array>("maps");
        Maps.reserve(maps.size());

        for (int32_t i = 0; i < maps.size(); i++)
        {
            jsonxx::Object map = maps.get<jsonxx::Object>(i);
            packageMapSpecMap.Name = map.get<jsonxx::String>("name");
            Maps.push_back(packageMapSpecMap);
        }
    }

    // Dump all PackageMapSpec data
    std::string PackageMapSpec::Dump() const
    {
        jsonxx::Object packageMapSpecJson;
        jsonxx::Array files;
        jsonxx::Array mapFileRefs;
        jsonxx::Array maps;

        for (auto& file : Files)
        {
            jsonxx::Object jsonFile;
            jsonFile << "name" << file.Name;
            files << jsonFile;
        }

        for (auto& mapFileRef : MapFileRefs)
        {
            jsonxx::Object jsonMapFileRef;
            jsonMapFileRef << "file" << mapFileRef.File;
            jsonMapFileRef << "map" << mapFileRef.Map;
            mapFileRefs << jsonMapFileRef;
        }

        for (auto& map : Maps)
        {
            jsonxx::Object jsonMap;
            jsonMap << "name" << map.Name;
            maps << jsonMap;
        }

        packageMapSpecJson << "files" << files;
        packageMapSpecJson << "mapFileRefs" << mapFileRefs;
        packageMapSpecJson << "maps" << maps;

        return packageMapSpecJson.json();
    }

    // Convert path separators to "/", to match packageMapSpec format
    void PackageMapSpec::NormalizeFilePath(std::string& filePath) const
    {
        std::replace(filePath.begin(), filePath.end(), '\\', '/');
        return;
    }

    // Confirm filepath contains "Base" directory
    bool PackageMapSpec::InBaseDirectory(const std::string& filePath) const
    {
        size_t strPos = filePath.rfind("base");
        if (strPos == -1)
            return 0;
        return 1;
    }

    // Returns filepath relative to "Base" directory
    std::string PackageMapSpec::GetRelativeFilePath(const std::string& filePath) const
    {
        size_t strPos = filePath.rfind("base");
        std::string relativePath = filePath.substr(strPos + 5, filePath.length() - strPos);
        return relativePath;
    }

    // Returns file index for a given filename
    size_t PackageMapSpec::GetFileIndexByFileName(const std::string& filePath) const
    {       
        auto fileIterator = std::find_if(Files.begin(), Files.end(), [&](const PackageMapSpecFile& file) { return file.Name == filePath; });
        if (fileIterator == Files.end())
            return -1;
        return std::distance(Files.begin(), fileIterator);
    }

    // Returns map index for a given file index
    size_t PackageMapSpec::GetMapIndexByFileIndex(const size_t fileIndex) const
    {
        auto mapFileRefIterator = std::find_if(MapFileRefs.begin(), MapFileRefs.end(),
            [&](const PackageMapSpecMapFileRef& mapFileRef) { return mapFileRef.File == fileIndex; });

        if (mapFileRefIterator == MapFileRefs.end())
            return -1;

        return MapFileRefs[std::distance(MapFileRefs.begin(), mapFileRefIterator)].Map;
    }

    // Returns list of .resources and .streamdbs loaded in a certain map
    std::vector<std::string> PackageMapSpec::GetFilesByResourceName(std::string resourceFileName) const
    {
        size_t fileIndex = -1;
        size_t mapIndex = -1;
        std::vector<std::string> fileList;

        NormalizeFilePath(resourceFileName);

        if (!InBaseDirectory(resourceFileName))
            return fileList;

        if (resourceFileName.rfind(".backup") != -1)
            resourceFileName = resourceFileName.substr(0, resourceFileName.rfind(".backup"));
        
        resourceFileName = GetRelativeFilePath(resourceFileName);
        fileIndex = GetFileIndexByFileName(resourceFileName);
        mapIndex = GetMapIndexByFileIndex(fileIndex);

        if (mapIndex == -1 || fileIndex == -1)
            return fileList;

        for (const auto& mapFileRef : MapFileRefs)
        {
            if (mapFileRef.Map != mapIndex)
                continue;
            fileList.push_back(Files[mapFileRef.File].Name);
        }
        return fileList;
    }
}