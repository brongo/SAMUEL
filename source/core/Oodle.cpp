#include "Oodle.h"

namespace HAYDEN {
    // Decompress using Oodle DLL
    OodLZ_DecompressFunc *OodLZ_Decompress = NULL;

    bool oodleInit(const std::string &basePath) {
        std::string oodlePath = basePath.substr(0, basePath.length() - 4) + "oo2core_8_win64.dll";

#ifdef _WIN32
        // Load oodle dll
        auto oodle = LoadLibraryA(oodlePath.c_str());
        if (!oodle)
            return false;

        OodLZ_Decompress = (OodLZ_DecompressFunc *) GetProcAddress(oodle, "OodleLZ_Decompress");
#else
        std::error_code ec;

        // Copy oodle to current dir to prevent linoodle errors
        bool copied = false;
        if (!fs::equivalent(fs::path(oodlePath).parent_path(), fs::current_path())) {
            fs::copy(oodlePath, fs::current_path(), fs::copy_options::overwrite_existing, ec);
            copied = true;
            if (ec.value() != 0)
                return false;
        }

        // Load linoodle library
        std::string linoodlePath = basePath + "/liblinoodle.so";
        auto oodle = dlopen(linoodlePath.c_str(), RTLD_LAZY);
        OodLZ_Decompress = (OodLZ_DecompressFunc*)dlsym(oodle, "OodleLZ_Decompress");

        // Remove oodle dll
        if (copied)
            fs::remove(fs::current_path().append("oo2core_8_win64.dll"), ec);
#endif

        if (oodle == nullptr || OodLZ_Decompress == nullptr)
            return false;

        return true;
    }

    std::vector<uint8_t> oodleDecompress(const std::vector<uint8_t> &compressedData, uint64_t decompressedSize) {
        if (OodLZ_Decompress == nullptr)
            return {};

        std::vector<uint8_t> output(decompressedSize + SAFE_SPACE);
        uint64_t outbytes = 0;

        // Decompress using Oodle DLL
        outbytes = OodLZ_Decompress(compressedData.data(), (int) compressedData.size(), output.data(), decompressedSize,
                                    0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, 0);

        if (outbytes == 0) {
            fprintf(stderr, "Error: failed to decompress with Oodle DLL.\n\n");
            return {};
        }

        return std::move(std::vector<uint8_t>(output.begin(), output.begin() + (int64_t) outbytes));
    }

    void oodleDecompressInplace(std::vector<uint8_t> &compressedData, uint64_t decompressedSize) {
        if (OodLZ_Decompress == nullptr)
            return;

        std::vector<uint8_t> output(decompressedSize + SAFE_SPACE);
        uint64_t outbytes = 0;

        // Decompress using Oodle DLL
        outbytes = OodLZ_Decompress(compressedData.data(), (int) compressedData.size(), output.data(), decompressedSize,
                                    0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, 0);

        if (outbytes == 0) {
            fprintf(stderr, "Error: failed to decompress with Oodle DLL.\n\n");
            return;
        }
        compressedData.clear();
        compressedData.resize(outbytes);
        compressedData.insert(compressedData.begin(), output.begin(), output.begin() + (int64_t) outbytes);
    }
}