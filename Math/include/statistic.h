#pragma once

#include <xmmintrin.h>

/// Compare and set maximum values
inline void maxSet(const __m128 &data, __m128 &max);

/// Compare and set minimum values
inline void minSet(const __m128 &data, __m128 &min);

/// Update provided mean values
inline void meanWalk(const __m128 &data,
                     const __m128 &oldmean,
                     const uint32_t n,
                     __m128 &mean);

/// Update provided std values
inline void stdWalk(const __m128 &data,
                    const __m128 &oldmean,
                    const __m128 &mean,
                    const __m128 &oldStd,
                    __m128 &Std);

/// Calculate final std values
inline void flushStd(const __m128 &data, const __m128 &n, float *out);