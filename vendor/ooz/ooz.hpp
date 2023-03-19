#include <cstdint>
#include <cstddef>
#include <vector>

extern "C" {
    int Kraken_Compress(uint8_t* src, size_t src_len, uint8_t* dst, int level);
    int Kraken_Decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len);
};
