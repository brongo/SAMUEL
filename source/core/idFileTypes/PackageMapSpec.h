#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "../../../vendor/jsonxx/jsonxx.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    class PackageMapSpec;

    class PackageMapSpecFile {
    public:
        [[nodiscard]] const std::string &name() const {
            return m_name;
        }

    private:
        std::string m_name;

        friend PackageMapSpec;
    };

    class PackageMapSpecMapFileRef {
    public:
        PackageMapSpecMapFileRef() {
            m_file = -1;
            m_map = -1;
        }

        PackageMapSpecMapFileRef(const int32_t file, const int32_t map) {
            m_file = file;
            m_map = map;
        }

        [[nodiscard]] int32_t file() const { return m_file; }

        [[nodiscard]] int32_t map() const { return m_map; }

    private:
        int32_t m_file;
        int32_t m_map;

        friend PackageMapSpec;
    };

    class PackageMapSpecMap {
    public:
        std::string m_name;
    };

    class PackageMapSpec {

    public:
        PackageMapSpec() = default;

        explicit PackageMapSpec(const std::string &json);

        [[nodiscard]] std::string dump() const;

        [[nodiscard]] static bool inBaseDirectory(const std::string &filePath);

        static std::string normalizedFilePath(const std::string &filePath);

        [[nodiscard]] size_t getFileIndexByFileName(const std::string &filePath) const;

        [[nodiscard]] size_t getMapIndexByFileIndex(size_t fileIndex) const;

        [[nodiscard]] static std::string getRelativeFilePath(const std::string &filePath);

        [[nodiscard]] std::vector<std::string> getFilesByResourceName(const std::string &resourceFileName) const;

        [[nodiscard]] std::vector<std::string> getFilesByResourceName(const fs::path resourceFileName) const {
            return getFilesByResourceName(resourceFileName.string());
        };

        const std::vector<PackageMapSpecFile> &files() { return m_files; };

        const std::vector<PackageMapSpecMapFileRef> &mapFileRefs() { return m_mapFileRefs; };

        const std::vector<PackageMapSpecMap> &maps() { return m_maps; };

        [[nodiscard]] bool loaded() const { return m_loaded; };

        void reset();

    private:
        std::vector<PackageMapSpecFile> m_files;
        std::vector<PackageMapSpecMapFileRef> m_mapFileRefs;
        std::vector<PackageMapSpecMap> m_maps;
        bool m_loaded = false;
    };
}