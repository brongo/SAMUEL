#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

#ifdef _MSC_VER
    #define HAYDEN_PACK(_Def) __pragma(pack(push, 1)) _Def; __pragma(pack(pop))
#else
    #define HAYDEN_PACK(_Def) _Def __attribute__((packed));
#endif

namespace HAYDEN {
    // hex <-> decimal conversions and endian functions
    template<typename T>
    std::string intToHex(const T num) {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +num;
        return stream.str();
    }

    uint64_t hexToInt64(const std::string &hex);

    void endianSwap(uint64_t &value);

    std::string getFilename(const std::string &filename);

    std::string getFilename(const fs::path &filename);;

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows
    bool mkpath(const fs::path &path);

    // Opens FILE* with long filepath, bypasses PATH_MAX limitations in Windows
    FILE *openLongFilePathWin32(const fs::path &path, const wchar_t *mode = L"wb");

    // Removes quotation marks from a string
    std::string stripQuotes(const std::string &str);

    // Writes data from memory to local filesystem. Return 1 on success.
    bool writeToFilesystem(const std::vector<uint8_t> &outData, const fs::path &outPath);

    bool writeToFilesystem(const std::vector<char> &outData, const fs::path &outPath);

    bool writeToFilesystem(const std::string &outData, const fs::path &outPath);

    // Reads whole file into vector
    bool readFile(const fs::path &path, std::vector<uint8_t> &out);

    std::string readFile(const fs::path &path);

    template<typename T>
    bool readT(std::ifstream &file, T &value) {
        static_assert(std::is_trivial_v<T>, "Type must be trivial");
        return !!file.read(reinterpret_cast<char *>(&value), sizeof(T));
    }

    bool createPaths(const fs::path& outputFile);
}
