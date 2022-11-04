#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace HAYDEN
{
    // hex <-> decimal conversions and endian functions
    template <typename T>
    std::string intToHex(const T num)
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +num;
        return stream.str();
    }
    uint64_t hexToInt64(const std::string hex);
    void endianSwap(uint64_t& value);

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows
    bool mkpath(const fs::path& path);

    // Opens FILE* with long filepath, bypasses PATH_MAX limitations in Windows
    FILE* openLongFilePathWin32(const fs::path& path);

    // Removes quotation marks from a string
    std::string stripQuotes(std::string str);

    // Writes data from memory to local filesystem. Return 1 on success.
    bool writeToFilesystem(std::vector<uint8_t> outData, fs::path outPath);

    // Reads whole file into vector
    bool readFile(const fs::path& path, std::vector<uint8_t>& out);
}