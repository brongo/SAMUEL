#include <iostream>
#include "Utilities.h"

namespace HAYDEN {
    void endianSwap(uint64_t &value) {
        value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
        value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
        value = ((value & 0x00FF00FF00FF00FFull) << 8) | ((value & 0xFF00FF00FF00FF00ull) >> 8);
    }

    uint64_t hexToInt64(const std::string &hex) {
        return std::stoull(hex, nullptr, 16);
    }

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows       
    bool mkpath(const fs::path &path) {
        std::error_code ec;
#ifdef _WIN32
        // "\\?\" alongside the wide string functions is used to bypass PATH_MAX
        // Check https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for details 
        fs::create_directories(L"\\\\?\\" + fs::absolute(path).wstring(), ec);
#else
        fs::create_directories(path, ec);
#endif
        return ec.value() == 0;
    }

#ifdef _WIN32

    // Opens FILE* with long filepath, bypasses PATH_MAX limitations in Windows
    FILE *openLongFilePathWin32(const fs::path &path, const wchar_t *mode) {
        // "\\?\" alongside the wide string functions is used to bypass PATH_MAX
        // Check https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for details
        std::wstring wPath = L"\\\\?\\" + path.wstring();
        FILE *file = nullptr;
        errno_t err = _wfopen_s(&file, wPath.c_str(), mode);
        return (err == 0) ? file : nullptr;
    }

#endif

    // Remove quotation marks from a string
    std::string stripQuotes(const std::string &str) {
        std::string result = str;
        result.erase(std::remove_if(result.begin(), result.end(),
                                    [](char c) { return c == '"' || c == '\''; }), result.end());
        return result;
    }

    std::filesystem::path replace_char_in_path(const std::filesystem::path &path, char old_char, char new_char) {
        std::string path_str = path.string(); // convert the path to a string

        // replace characters in the string
        std::replace(path_str.begin(), path_str.end(), old_char, new_char);

        // convert the string back to a path
        std::filesystem::path new_path(path_str);

        return new_path;
    }

    bool writeToFilesystem(const std::vector<char> &outData, const fs::path &outPath) {
        std::vector<uint8_t> data(outData.begin(), outData.end());
        return writeToFilesystem(data, outPath);
    }

    bool writeToFilesystem(const std::string &outData, const fs::path &outPath) {
        std::vector<uint8_t> data(outData.begin(), outData.end());
        return writeToFilesystem(data, outPath);
    }

    bool writeToFilesystem(const std::vector<uint8_t> &outData, const fs::path &outPath) {
        fs::path folderPath = outPath;
        folderPath.remove_filename();
        fs::path tmpOutPath = replace_char_in_path(outPath, '/', '\\');

        if (!fs::exists(folderPath)) {
            if (!mkpath(folderPath)) {
                fprintf(stderr, "Error: Failed to create directories for file: %s \n", tmpOutPath.string().c_str());
                return false;
            }
        }
        std::cout << "Writing " << outPath << std::endl;
        // open file for writing
#ifdef _WIN32
        FILE *outFile = openLongFilePathWin32(tmpOutPath); //wb
#else
        FILE* outFile = fopen(tmpOutPath.string().c_str(), "wb");
#endif

        if (outFile == nullptr) {
            fprintf(stderr, "Error: Failed to open file for writing: %s \n", tmpOutPath.string().c_str());
            return false;
        }

        fwrite(outData.data(), 1, outData.size(), outFile);
        fclose(outFile);
        return true;
    }

    // Reads whole file into vector
    bool readFile(const fs::path &path, std::vector<uint8_t> &out) {
        std::ifstream f(path, std::ios_base::in | std::ios_base::binary);

        if (!f)
            return false;

        f.seekg(0, std::ios_base::end);
        std::streamsize length = f.tellg();
        f.seekg(0, std::ios_base::beg);

        out.resize(length);

        if (!f.read(reinterpret_cast<char *>(out.data()), length)) {
            out.clear();
            return false;
        }

        return true;
    }

    std::string readFile(const fs::path &path) {
        std::ifstream f(path, std::ios_base::in | std::ios_base::binary);
        std::string out;

        if (!f)
            return std::move(out);

        f.seekg(0, std::ios_base::end);
        std::streamsize length = f.tellg();
        f.seekg(0, std::ios_base::beg);
        out.resize(length);

        if (!f.read(reinterpret_cast<char *>(out.data()), length)) {
            out.clear();
            return std::move(out);
        }

        return std::move(out);
    }

    std::string HAYDEN::getFilename(const std::string &filename) {
        auto sepIndex = std::find(filename.begin(), filename.end(), '$');
        std::string tmp = filename.substr(0, std::distance(filename.begin(), sepIndex));
        return fs::path(tmp).filename().string();
    }

    std::string getFilename(const fs::path &filename) { return getFilename(filename.string()); }

    bool createPaths(const fs::path &outputFile) {
        if (!fs::exists(outputFile)) {
            if (!mkpath(outputFile)) {
                fprintf(stderr, "Error: Failed to create directories for file: %s \n",
                        outputFile.string().c_str());
                return false;
            }
        }
        return true;
    }
}