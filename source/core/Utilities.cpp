#include "Utilities.h"

namespace HAYDEN
{
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

    // Decompress using Oodle DLL
    OodLZ_DecompressFunc* OodLZ_Decompress = NULL;

    bool oodleInit(const std::string& basePath)
    {
#ifdef _WIN32
        std::string oodlePath = basePath.substring(0, basePath.length() - 4) + "oo2core_8_win64.dll";
        auto oodle = LoadLibraryA(oodlePath.c_str());
        OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(oodle, "OodleLZ_Decompress");
#else
        std::string linoodlePath = basePath + "/liblinoodle.so";
        auto oodle = dlopen(linoodlePath.c_str(), RTLD_LAZY);
        OodLZ_Decompress = (OodLZ_DecompressFunc*)dlsym(oodle, "OodleLZ_Decompress");
#endif

        if (oodle == NULL || OodLZ_Decompress == NULL)
            return false;

        return true;
    }
    std::vector<byte> oodleDecompress(std::vector<byte> compressedData, const uint64 decompressedSize)
    {
        if (OodLZ_Decompress == NULL)
            return std::vector<byte>();

        std::vector<byte> output(decompressedSize + SAFE_SPACE);
        uint64 outbytes = 0;

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
}
