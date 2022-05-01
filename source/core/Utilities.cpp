#include "Utilities.h"

namespace HAYDEN
{
    void endianSwap(uint64_t& value)
    {
        value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
        value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
        value = ((value & 0x00FF00FF00FF00FFull) << 8) | ((value & 0xFF00FF00FF00FF00ull) >> 8);
    }

    uint64_t hexToInt64(const std::string hex)
    {
        uint64_t x;
        std::stringstream stream;
        stream << std::hex << hex;
        stream >> x;
        return x;
    }

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows       
    bool mkpath(const fs::path& path)
    {
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
    FILE* openLongFilePathWin32(const fs::path& path)
    {
        // "\\?\" alongside the wide string functions is used to bypass PATH_MAX
        // Check https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for details 
        std::wstring wPath = L"\\\\?\\" + path.wstring();
        FILE* file = _wfopen(wPath.c_str(), L"wb");
        return file;
    }
    #endif

    // Remove quotation marks from a string
    std::string stripQuotes(std::string str)
    {
        std::string::iterator end_pos = std::remove(str.begin(), str.end(), '"');
        str.erase(end_pos, str.end());
        std::string::iterator end_pos2 = std::remove(str.begin(), str.end(), '\'');
        str.erase(end_pos2, str.end());
        return str;
    }

    // Writes data from memory to local filesystem. Return 1 on success.
    bool writeToFilesystem(std::vector<uint8_t> outData, fs::path outPath)
    {
        fs::path folderPath = outPath;
        folderPath.remove_filename();

        if (!fs::exists(folderPath))
        {
            if (!mkpath(folderPath))
            {
                fprintf(stderr, "Error: Failed to create directories for file: %s \n", outPath.string().c_str());
                return 0;
            }
        }

        // open file for writing
#ifdef _WIN32
        FILE* outFile = openLongFilePathWin32(outPath); //wb
#else
        FILE* outFile = fopen(outPath.string().c_str(), "wb");
#endif

        if (outFile == NULL)
        {
            fprintf(stderr, "Error: Failed to open file for writing: %s \n", outPath.string().c_str());
            return 0;
        }

        fwrite(outData.data(), 1, outData.size(), outFile);
        fclose(outFile);        
        return 1;
    }

    // Reads whole file into vector
    bool readFile(const fs::path& path, std::vector<uint8_t>& out)
    {
        FILE *f = fopen(path.string().c_str(), "rb");
        if (f == NULL)
            return false;

        fseek(f, 0, SEEK_END);
        size_t length = ftell(f);
        fseek(f, 0, SEEK_SET);
        out.resize(length);

        fread(out.data(), 1, length, f);
        fclose(f);

        return true;
    }
}