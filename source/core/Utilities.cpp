#ifndef _WIN32
#include <dlfcn.h>
#endif

#include "Utilities.h"

namespace HAYDEN
{
    // hex <-> decimal conversions and endian functions
    void endianSwap(uint64& value) 
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

    // oodle functions
    std::vector<byte> oodleDecompress(std::vector<byte> compressedData, const uint64 decompressedSize)
    {
        std::vector<byte> output(decompressedSize + SAFE_SPACE);
        uint64 outbytes = 0;
        OodLZ_DecompressFunc* OodLZ_Decompress;

#ifdef _WIN32
        auto oodle = LoadLibraryA("./oo2core_8_win64.dll");
        OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(oodle, "OodleLZ_Decompress");
#else
        auto oodle = dlopen("./liblinoodle.so", RTLD_LAZY);
        OodLZ_Decompress = (OodLZ_DecompressFunc*)dlsym(oodle, "OodleLZ_Decompress");

        if (dlerror() != NULL)
        {
            fprintf(stderr, "Error: failed to load oo2core_8_win64.dll.\n\n");
            return std::vector<byte>();
        }
#endif

        if (OodLZ_Decompress == NULL)
        {
            fprintf(stderr, "Error: failed to load oo2core_8_win64.dll.\n\n");
            return std::vector<byte>();
        }

        // Decompress using Oodle DLL
        outbytes = OodLZ_Decompress(compressedData.data(), compressedData.size(), output.data(), decompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        
        if (outbytes == 0)
        {
            fprintf(stderr, "Error: failed to decompress with Oodle DLL.\n\n");
            return std::vector<byte>();
        }

        return std::vector<byte>(output.begin(), output.begin() + outbytes);
    }

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows
    bool mkpath(const fs::path& path)
    {
#ifdef __linux__
        // Use filesystem function on Linux
        return fs::create_directories(path);
#else
        // Get full path in wide string - needed for PATH_MAX bypass
        std::wstring dirPath = fs::absolute(path).wstring();
        std::vector<std::wstring> dirPathSections = { dirPath };
        size_t backwardSlashIndex;

        // Get all directories to make
        // For example, if the path is "C:\\foo\bar" the directories to create are "C:\", "C:\foo", "C:\foo\bar"
        while ((backwardSlashIndex = dirPath.rfind('\\')) != std::string::npos) {
            dirPath = dirPath.substr(0, backwardSlashIndex);
            dirPathSections.push_back(dirPath);
        }

        // Reverse directory order to start from lowest
        std::reverse(dirPathSections.begin(), dirPathSections.end());

        // Make directories
        for (auto &path : dirPathSections) {
            if (fs::is_directory(path))
                continue;

            // "\\?\" alongside the wide string functions is used to bypass PATH_MAX
            // Check https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for details
            if (_wmkdir((L"\\\\?\\" + path).c_str()) == -1 && errno != EEXIST)
                return 0;
        }

        return 1;
#endif
    }
}
