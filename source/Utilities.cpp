#ifdef __linux__
#include <dlfcn.h>
#endif

#include "Utilities.h"

namespace HAYDEN
{
    // filepath parse functions
    std::string getPathWithoutFileName(std::string filename)
    {
        return filename.substr(0, filename.rfind(fs::path::preferred_separator));
    }
    std::string getFilenameFromPath(std::string path)
    {
        size_t splitPos = path.rfind(fs::path::preferred_separator);
        return path.substr(splitPos + 1, path.length() - splitPos);
    }
    std::string getParentFolderFromPath(std::string path)
    {
        return path.substr(0, path.find(fs::path::preferred_separator));
    }
    std::string dropFileExtension(std::string filename)
    {
        return filename.substr(0, filename.rfind("."));
    }
    std::string dropFirstDirectoryFromPath(std::string path)
    {
        size_t splitPos = path.find(fs::path::preferred_separator);
        return path.substr(splitPos + 1, path.length() - splitPos);
    }

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
    std::vector<byte> intToByteArray(int x)
    {
        std::vector<byte> bytes(reinterpret_cast<byte*>(&x),
            reinterpret_cast<byte*>(&x) + sizeof x);
        return bytes;
    }
    // oodle functions
    std::vector<byte> oodleDecompress(std::vector<byte> compressedData, const uint64 decompressedSize)
    {
        std::vector<byte> output(decompressedSize + SAFE_SPACE);
        uint64 outbytes;
        OodLZ_DecompressFunc* OodLZ_Decompress;

        #ifdef _WIN32
            auto oodle = LoadLibraryA("./oo2core_8_win64.dll");
            OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(oodle, "OodleLZ_Decompress");
        #else
            auto oodle = dlopen("./liblinoodle.so", RTLD_LAZY);
            OodLZ_Decompress = (OodLZ_DecompressFunc*)dlsym(oodle, "OodleLZ_Decompress");
        #endif

        if (!OodLZ_Decompress)
        {
            fprintf(stderr, "Error: failed to load oo2core_8_win64.dll.\n\n");
            return std::vector<byte>();
        }

        // Decompress using Oodle DLL
        outbytes = OodLZ_Decompress(compressedData.data(), compressedData.size(), output.data(), decompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        
        if (!outbytes)
        {
            fprintf(stderr, "Error: failed to decompress with Oodle DLL.\n\n");
            return std::vector<byte>();
        }

        return std::vector<byte>(output.begin(), output.begin() + outbytes);
    }
}