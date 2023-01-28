#include "Oodle.h"

namespace HAYDEN
{
#ifdef _WIN32
    // Decompress using Oodle DLL
    OodLZ_DecompressFunc* OodLZ_Decompress = NULL;
#else
    // Decompress using ooz library
    Kraken_Decompress* KrakenDecompress = NULL;
#endif

    bool oodleInit(const std::string& basePath)
    {
#ifdef _WIN32
        // Load oodle dll
        std::string oodlePath = basePath.substr(0, basePath.length() - 4) + "oo2core_8_win64.dll";
        auto oodle = LoadLibraryA(oodlePath.c_str());
        if (!oodle)
            return false;

        OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(oodle, "OodleLZ_Decompress");
        if (!OodLZ_Decompress)
            return false;
#else
        // Load ooz library
        std::string oozPath = basePath + "/libooz.so";
        auto ooz = dlopen(oozPath.c_str(), RTLD_LAZY);
        if (!ooz)
            return false;

        KrakenDecompress = (Kraken_Decompress*)dlsym(ooz, "Kraken_Decompress");
        if (!KrakenDecompress)
            return false;
#endif

        return true;
    }

    std::vector<uint8_t> oodleDecompress(std::vector<uint8_t> compressedData, const uint64_t decompressedSize)
    {
#ifdef _WIN32
        if (OodLZ_Decompress == NULL)
            return std::vector<uint8_t>();
#else
        if (KrakenDecompress == NULL)
            return std::vector<uint8_t>();
#endif

        std::vector<uint8_t> output(decompressedSize + SAFE_SPACE);
        uint64_t outbytes = 0;

#ifdef _WIN32
        // Decompress using Oodle DLL
        outbytes = OodLZ_Decompress(compressedData.data(), compressedData.size(), output.data(), decompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#else
        // Decompress using ooz library
        outbytes = KrakenDecompress(compressedData.data(), compressedData.size(), output.data(), decompressedSize);
#endif

        if (outbytes == 0)
        {
            fprintf(stderr, "Error: failed to decompress with Oodle DLL.\n\n");
            return std::vector<uint8_t>();
        }

        return std::vector<uint8_t>(output.begin(), output.begin() + outbytes);
    }
}