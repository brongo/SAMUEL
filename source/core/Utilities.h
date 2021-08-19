#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <filesystem>

#define SAFE_SPACE 64

typedef unsigned char byte;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int64_t int64;

typedef int OodLZ_DecompressFunc(
    byte* src_buf, int src_len, byte* dst, size_t dst_size, int fuzz, int crc, int verbose,
    byte* dst_base, size_t e, void* cb, void* cb_ctx, void* scratch, size_t scratch_size, int threadPhase);

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
    void endianSwap(uint64& value); 

    // Decompress using Oodle DLL
    std::vector<byte> oodleDecompress(std::vector<byte> compressedData, const uint64 decompressedSize);

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows
    bool mkpath(const fs::path& path);

    // Opens FILE* with long filepath, bypasses PATH_MAX limitations in Windows
    FILE* openLongFilePathWin32(const fs::path& path);
}
