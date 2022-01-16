#include "crc.h"

uint64_t byte_crc32(unsigned int crc, const char **buf, size_t len)
{
    while (len > 0)
    {
        crc = _mm_crc32_u8(crc, *(const unsigned char *)(*buf));
        ++*buf;
        --len;
    }

    return crc;
}

uint64_t hw_crc32(unsigned int crc, const char **buf, size_t len)
{
    while (len > 0)
    {
        crc = _mm_crc32_u16(crc, *(const uint16_t *)(*buf));
        *buf += 2;
        --len;
    }

    return crc;
}

uint64_t word_crc32(unsigned int crc, const char **buf, size_t len)
{
    while (len > 0)
    {
        crc = _mm_crc32_u32(crc, *(const uint32_t *)(*buf));
        *buf += 4;
        --len;
    }

    return crc;
}

uint64_t dword_crc32(uint64_t crc, const char **buf, size_t len)
{
    while (len > 0)
    {
        crc = _mm_crc32_u64(crc, *(const uint64_t *)(*buf));
        *buf += 8;
        --len;
    }

    return crc;
}

uint64_t crc(const char *buff, size_t len)
{
    const size_t dword_chunks = len / 8;
    const size_t dword_diff = len % 8;

    const size_t word_chunks = dword_diff / 4;
    const size_t word_diff = dword_diff % 4;

    const size_t hw_chunks = word_diff / 2;
    const size_t hw_diff = word_diff % 2;

    uint64_t crc = byte_crc32(0, &buff, hw_diff);
    crc = hw_crc32(crc, &buff, hw_chunks);
    crc = word_crc32(crc, &buff, word_chunks);
    crc = dword_crc32(crc, &buff, dword_chunks);

    return crc;
}