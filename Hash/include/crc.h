#pragma once

#include <string>
#include <inttypes.h>
#include <nmmintrin.h>

/**
 * @brief               Computes CRC32 hash using intrinsic functions
 * 
 * @param buff          Pointer to data buff
 * @param len           Length of the data
 * @return uint64_t     Calculated CRC32 hash
 */
uint64_t crc(const char *buff, size_t len);

class CRCHashFunction
{
public:
    size_t operator()(const std::string &p) const
    {
        return crc(p.c_str(), p.size());
    }

    size_t operator()(const void *p, size_t len) const
    {
        return crc(p, len);
    }
};