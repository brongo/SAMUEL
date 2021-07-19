#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#define SAFE_SPACE 64

typedef unsigned char byte;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int64_t int64;

typedef int OodLZ_DecompressFunc(
    byte* src_buf, int src_len, byte* dst, size_t dst_size, int fuzz, int crc, int verbose,
    byte* dst_base, size_t e, void* cb, void* cb_ctx, void* scratch, size_t scratch_size, int threadPhase);

namespace HAYDEN
{
    template <typename T>
    std::string intToHex(T num)
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +num;
        return stream.str();
    }

    uint64_t hexToInt64(std::string hex); 
    void endianSwap(uint64& value); 
    
    bool oodleDecompress(const char* outputFile, byte* compressedData, uint64 compressedSize, uint64 decompressedSize);
}
