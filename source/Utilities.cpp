#include "Utilities.h"

namespace HAYDEN
{
    void endianSwap(std::uint64_t& value) 
    {
        value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
        value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
        value = ((value & 0x00FF00FF00FF00FFull) << 8) | ((value & 0xFF00FF00FF00FF00ull) >> 8);
    }

    std::string int64ToHex(uint64_t num)
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(sizeof(uint64_t) * 2) << std::hex << +num;
        return stream.str();
    }

    uint64_t hexToInt64(std::string hex) 
    {
        uint64_t x;
        std::stringstream stream;
        stream << std::hex << hex;
        stream >> x;
        return x;
    }

    bool oodleDecompress(const char* outputFile, byte* compressedData, uint64 compressedSize, uint64 decompressedSize)
    {
        FILE* f;
        byte* output;
        uint64 outbytes;

        OodLZ_DecompressFunc* OodLZ_Decompress;
        auto oodle = LoadLibraryA("./oo2core_8_win64.dll");

        OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(oodle, "OodleLZ_Decompress");
        if (!OodLZ_Decompress)
        {
            printf("Error: failed to load oo2core_8_win64.dll.\n\n");
            return 0;
        }

        // Decompress using Oodle DLL
        output = new byte[decompressedSize + SAFE_SPACE];
        outbytes = OodLZ_Decompress(compressedData, compressedSize, output, decompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        
        if (!outbytes)
        {
            printf("Error: failed to decompress with Oodle DLL.\n\n");
            return 0;
        }

        if (fopen_s(&f, outputFile, "wb") != 0)
        {
            printf("Error: failed to open destination file for writing.\n\n");
            return 0;
        }

        // Write to file
        if (f != NULL)
        {
            fwrite(output, 1, outbytes, f);
            fclose(f);
        }
        return 1;
    }
}