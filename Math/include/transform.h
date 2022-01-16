#pragma once

#include <stdio.h>
#include <math.h>

#include <omp.h>

#include <ipp.h>
#include <mkl.h>

#define DBLIMIT -150

/// Init Hilbert Transform
inline IppStatus createhilbert(IppsHilbertSpec *&hilbert, Ipp8u *&pBuffer, const size_t wlen);
/// Calculate Hilbert transform
inline IppStatus calchilbert(IppsHilbertSpec *hilbert,
                              Ipp8u *pBuffer,
                              const double *data,
                              double *output,
                              const size_t wlen);

/// Init FFT (MKL)
inline long createFFT_MKL(DFTI_DESCRIPTOR_HANDLE *fft, const int wlen);
/// Calculate FFT (MKL)
double *calculateFFT_MKL(double *data, double *window, int dataLen, DFTI_DESCRIPTOR_HANDLE *fft);

/// Calculate spectrogram
long spectrogram(double* data, int dataLen, double*** output, double* window, int wlen, int overlap, int bits);

/// Circle intersection
inline double **cirction(const double x0,
                         const double y0,
                         double *r0,
                         const double x1,
                         const double y1,
                         double *r1,
                         size_t *n);