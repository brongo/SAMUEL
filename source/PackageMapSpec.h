#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "../vendor/jsonxx/jsonxx.h"

namespace HAYDEN
{
    class PackageMapSpecFile
    {
        public:
            std::string Name;
    };

    class PackageMapSpecMapFileRef
    {
        public:
            int32_t File;
            int32_t Map;

            PackageMapSpecMapFileRef()
            {
                File = -1;
                Map = -1;
            }

            PackageMapSpecMapFileRef(int32_t file, int32_t map)
            {
                File = file;
                Map = map;
            }
    };

    class PackageMapSpecMap
    {
        public:
            std::string Name;
    };

    class PackageMapSpec
    {
        public:
            std::vector<PackageMapSpecFile> Files;
            std::vector<PackageMapSpecMapFileRef> MapFileRefs;
            std::vector<PackageMapSpecMap> Maps;

        public:
            PackageMapSpec() {};
            PackageMapSpec(std::string& json);
            std::string Dump();

            bool InBaseDirectory(std::string& filePath); 
            void NormalizeFilePath(std::string& filePath);
            size_t GetFileIndexByFileName(std::string& filePath);
            size_t GetMapIndexByFileIndex(size_t fileIndex);

            std::string GetRelativeFilePath(std::string& filePath);
            std::vector<std::string> GetFilesByResourceName(std::string resourceFileName);
    };
}