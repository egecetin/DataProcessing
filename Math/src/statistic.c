#include "statistic.h"

inline void maxSet(const __m128 &data, __m128 &max)
{
    __m128 mask = _mm_cmpgt_ps(data, max);
    __m128 buff1 = _mm_and_ps(data, mask);
    __m128 buff2 = _mm_andnot_ps(mask, max);
    max = _mm_add_ps(buff1, buff2);
    return;
}

inline void minSet(const __m128 &data, __m128 &min)
{
    __m128 mask = _mm_cmplt_ps(data, min);
    __m128 buff1 = _mm_and_ps(data, mask);
    __m128 buff2 = _mm_andnot_ps(mask, min);
    min = _mm_add_ps(buff1, buff2);
    return;
}

inline void meanWalk(const __m128 &data,
                     const __m128 &oldmean,
                     const uint32_t n,
                     __m128 &mean)
{
    mean = _mm_sub_ps(data, oldmean);
    mean = _mm_div_ps(mean, _mm_set1_ps(n));
    mean = _mm_add_ps(mean, oldmean);
    return;
}

inline void stdWalk(const __m128 &data,
                    const __m128 &oldmean,
                    const __m128 &mean,
                    const __m128 &oldStd,
                    __m128 &Std)
{
    __m128 buff1 = _mm_sub_ps(data, oldmean);
    __m128 buff2 = _mm_sub_ps(data, mean);
    Std = _mm_mul_ps(buff1, buff2);
    Std = _mm_add_ps(Std, oldStd);
    return;
}

inline void flushStd(const __m128 &data, const __m128 &n, float *out)
{
    __m128 buff = _mm_div_ps(data, _mm_sub_ps(n, _mm_set1_ps(1)));
    buff = _mm_sqrt_ps(buff);
    _mm_store_ps(out, buff);
    return;
}