#include "transform.h"

inline IppStatus createhilbert(IppsHilbertSpec *&hilbert, Ipp8u *&pBuffer, const size_t wlen)
{
    IppStatus status = ippStsNoErr;
    int sizeSpec, sizeBuf;

    status = ippsHilbertGetSize_32f32fc(wlen, ippAlgHintNone, &sizeSpec, &sizeBuf);
    if (status != ippStsNoErr)
    {
        return status;
    }

    // Configure descriptor
    hilbert = (IppsHilbertSpec *)ippMalloc(sizeSpec);
    pBuffer = (Ipp8u *)ippMalloc(sizeBuf);
    if (hilbert == NULL || pBuffer == NULL)
    {    // Check allocated memory
        status = ippStsNoMemErr;
    }

    return status;
}

inline IppStatus calchilbert(IppsHilbertSpec *hilbert,
                              Ipp8u *pBuffer,
                              const double *data,
                              double *output,
                              const size_t wlen)
{
    IppStatus status = ippStsNoErr;
    float *fbuff      = (float *)mkl_malloc(sizeof(float) * wlen, 64);
    Ipp32fc *hbuff    = (Ipp32fc *)mkl_malloc(sizeof(Ipp32fc) * wlen, 64);

    if (fbuff == NULL || hbuff == NULL)
    {    // Check allocated memory
        status = ippStsNoMemErr;
        goto cleanup;
    }

    // Initialize
    status = ippsHilbertInit_32f32fc(wlen, ippAlgHintNone, hilbert, pBuffer);
    if (status != ippStsNoErr)
    {
        goto cleanup;
    }

    // Hilbert transform
    status = ippsConvert_64f32f(data, fbuff, wlen);    // Change precision
    if (status != ippStsNoErr)
    {
        goto cleanup;
    }
    status = ippsHilbert_32f32fc(fbuff, hbuff, hilbert, pBuffer);    // Hilbert transform
    if (status != ippStsNoErr)
    {
        goto cleanup;
    }
    status = ippsImag_32fc(hbuff, fbuff, wlen);    // Get imaginary part
    if (status != ippStsNoErr)
    {
        goto cleanup;
    }

    status = ippsConvert_32f64f(fbuff, output, wlen);    // Change precision
    if (status != ippStsNoErr)
    {
        goto cleanup;
    }

cleanup:
    mkl_free(fbuff);
    mkl_free(hbuff);
    return status;
}

long createFFT_MKL(DFTI_DESCRIPTOR_HANDLE *fft, const int wlen)
{
    long status = DFTI_NO_ERROR;

    status = DftiCreateDescriptor_d_1d(fft, DFTI_REAL, wlen);    // Create DFT Descriptor
    if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
        return status;
    status = DftiSetValue(*fft, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX);    // CCE Storage type
    if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
        return status;
    status = DftiSetValue(*fft, DFTI_PLACEMENT, DFTI_NOT_INPLACE);    // Out-of-place (Not replace input)
    if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
        return status;
    return DftiSetValue(*fft, DFTI_FORWARD_SCALE, 1.0 / wlen);    // Set forward scale 1/wlen
}

double *calculateFFT_MKL(double *data, double *window, int dataLen, DFTI_DESCRIPTOR_HANDLE *fft)
{
    long status = DFTI_NO_ERROR;

    double *buff         = (double *)MKL_malloc(dataLen * sizeof(double), 64);
    double *out          = (double *)MKL_malloc(dataLen / 2 * sizeof(double), 64);
    MKL_Complex16 *fdata = (MKL_Complex16 *)MKL_malloc((dataLen / 2 + 1) * sizeof(MKL_Complex16), 64);
    if (!(buff && fdata && out))
    {    // Check memory
        status = DFTI_MEMORY_ERROR;
        goto cleanup;
    }

    vdMul(dataLen, data, window, buff);    // Multiply with window

    status = DftiCommitDescriptor(*fft);    // Commit DFT
    if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
        goto cleanup;
    status = DftiComputeForward(*fft, buff, fdata);    // Compute DFT
    if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
        goto cleanup;

    vzAbs(dataLen / 2 + 1, fdata, out);    // Find magnitude

cleanup:
    if (status)
    {
        MKL_free(out);
        out = NULL;
    }

    MKL_free(buff);
    MKL_free(fdata);

    return out;
}

long spectrogram(double* data, int dataLen, double*** output, double* window, int wlen, int overlap, int bits)
{
	long status = DFTI_NO_ERROR;			// Error descriptor	

	int nth = MKL_Set_Num_Threads_Local(1);		// Set MKL thread to 1
	int max_thread = omp_get_max_threads();		// Get max thread num
	
	int shift = wlen - overlap;
	int outLen = floor((dataLen - wlen) / (shift)) + 1;
	DFTI_DESCRIPTOR_HANDLE *vfft = (DFTI_DESCRIPTOR_HANDLE *)MKL_malloc(max_thread * sizeof(DFTI_DESCRIPTOR_HANDLE), 64);	// FFT Descriptors

	// Init output	
	double **out = (double**)MKL_malloc(outLen * sizeof(double*), 64);
	if (!(out && vfft)) { // Check allocated memory
		status = DFTI_MEMORY_ERROR;
		goto cleanup;
	}
	for (int i = 0; i < outLen; ++i) {
		out[i] = (double*)MKL_malloc((wlen / 2 + 1) * sizeof(double), 64);
		if (!out[i]) { // Check allocated memory
			status = DFTI_MEMORY_ERROR;
			goto cleanup;
		}
	}

	// Init FFT
	for (int i = 0; i < max_thread; ++i) {
		status = createFFT_MKL(&vfft[i], wlen);
		if (status && !DftiErrorClass(status, DFTI_NO_ERROR)) {
			goto cleanup;
		}
	}

	// Compute FFT
	#pragma omp parallel for
	for (int i = 0; i < outLen; ++i) {
		if (!status) {
			IppStatus status_local = DFTI_NO_ERROR;
			int id = omp_get_thread_num();

			double* ptr = NULL;
			double* buff = (double*)MKL_malloc(wlen * sizeof(double), 64);
			MKL_Complex16* fdata = (MKL_Complex16*)MKL_malloc((wlen / 2 + 1) * sizeof(MKL_Complex16), 64);
			if (!(buff && fdata)) {	// Check memory
				#pragma omp critical
				status = DFTI_MEMORY_ERROR;
				continue;
			}

			vdMul(wlen, &data[i*shift], window, buff);	// Multiply with window
			if (status_local = vmlGetErrStatus(), status_local) {
				#pragma omp critical
				status = status_local;
				continue;
			}

			status_local = DftiCommitDescriptor(vfft[id]); // Commit DFT
			if (status_local && !DftiErrorClass(status_local, DFTI_NO_ERROR)) {
				#pragma omp critical
				status = status_local;
				continue;
			}

			status_local = DftiComputeForward(vfft[id], buff, fdata); // Compute DFT
			if (status_local && !DftiErrorClass(status_local, DFTI_NO_ERROR)) {
				#pragma omp critical
				status = status_local;
				continue;
			}

			vzAbs(wlen / 2 + 1, fdata, out[i]); // Find magnitude
			if (status_local = vmlGetErrStatus(), status_local) {
				#pragma omp critical
				status = status_local;
				continue;
			}

			status_local = ippsAddC_64f_I(0.0001, out[i], wlen / 2 + 1); // Add eps
			if (status_local)
			{
				#pragma omp critical
				status = status_local;
				continue;
			}

			cblas_dscal(wlen / 2 + 1, pow(2, -bits), out[i], 1); // Normalize
			vdLog10(wlen / 2 + 1, out[i], out[i]); // Log
			if (status_local = vmlGetErrStatus(), status_local) {
				#pragma omp critical
				status = status_local;
				continue;
			}
			cblas_dscal(wlen / 2 + 1, 20, out[i], 1); // Multiply 20

			ptr = out[i];
			for (int j = 0; j < wlen / 2 + 1; ++j) {
				if (ptr[j] < DBLIMIT)
					ptr[j] = DBLIMIT;
			}			

			// Free buffers
			MKL_free(fdata);
			MKL_free(buff);
		}		
	}

	if (max_thread == 1)	// All PCs have more than 1 thread if it is 1 there is a problem (Assume as warning)
		status = DFTI_MULTITHREADED_ERROR;
	
cleanup:
	MKL_Set_Num_Threads_Local(nth); // Set thread to default
	if (status) {
		for (int i = 0; i < outLen; ++i)
			MKL_free(out[i]);
		MKL_free(out);
		out = NULL;
	}
	*output = out;

	// Release FFT
	for (int i = 0; i < max_thread; ++i)
		status = DftiFreeDescriptor(&vfft[i]);
	MKL_free(vfft);

	return status;
}

inline double **cirction(const double x0,
                         const double y0,
                         double *r0,
                         const double x1,
                         const double y1,
                         double *r1,
                         size_t *n)
{
    IppStatus status = DFTI_MEMORY_ERROR;
    size_t k          = 0;
    const double d    = sqrt((y1 - y0) * (y1 - y0) + (x1 - x0) * (x1 - x0));    // Distance between circles
    double *r0_buff   = (double *)mkl_malloc((*n) * sizeof(double), 64);        // r0 Buffer
    double *r1_buff   = (double *)mkl_malloc((*n) * sizeof(double), 64);        // r1 Buffer

    // Copy data to buffer
    cblas_dcopy((*n), r0, 1, r0_buff, 1);
    cblas_dcopy((*n), r1, 1, r1_buff, 1);

    for (size_t i = 0; i < *n; ++i)
    {
        if (d > r0[i] + r1[i] || d < fabs(r0[i] - r1[i]) || d < 1e-6)
            continue;
        r0[k] = r0[i];
        r1[k] = r1[i];
        ++k;
    }
    *n = k;

    double *a   = (double *)mkl_malloc((*n) * sizeof(double), 64);    // Distance to center point
    double *h   = (double *)mkl_malloc((*n) * sizeof(double), 64);    // Distance between center point and intersection
    double *p2x = (double *)mkl_malloc((*n) * sizeof(double), 64);    // Center point x
    double *p2y = (double *)mkl_malloc((*n) * sizeof(double), 64);    // Center point y
    double **output = (double **)mkl_malloc(2 * sizeof(double *), 64);
    if (output != NULL)
    {
        output[0] = (double *)mkl_malloc(2 * (*n) * sizeof(double), 64);    // Output x coordinates
        output[1] = (double *)mkl_malloc(2 * (*n) * sizeof(double), 64);    // Output y coordinates
    }
    if (*n == 0)
        goto cleanup;

    if (a == NULL || h == NULL || p2x == NULL || p2y == NULL || output == NULL || output[0] == NULL ||
        output[1] == NULL)
    {    // Check allocated memory
        status = DFTI_MEMORY_ERROR;
        goto cleanup;
    }

    // Calculation a
    vdSqr((*n), r0, output[0]);
    status = vmlGetErrStatus();    // r0^2
    if (status != VML_STATUS_OK)
        goto cleanup;
    vdSqr((*n), r1, output[1]);
    status = vmlGetErrStatus();    // r1^2
    if (status != VML_STATUS_OK)
        goto cleanup;
    vdSub((*n), output[0], output[1], a);
    status = vmlGetErrStatus();    // r0^2-r1^2
    if (status != VML_STATUS_OK)
        goto cleanup;
    cblas_dscal((*n), 1 / (2 * d), a, 1);          // (r0^2-r1^2)/2d
    status = ippsAddC_64f(a, 0.5 * d, a, (*n));    // a=(r0^2-r1^2)/2d+d/2
    if (status != ippStsNoErr)
        goto cleanup;

    // Calculation h
    vdSqr((*n), a, output[1]);
    status = vmlGetErrStatus();    // a^2
    if (status != VML_STATUS_OK)
        goto cleanup;
    vdSub((*n), output[0], output[1], h);
    status = vmlGetErrStatus();    // h^2=r0^2-a^2
    if (status != VML_STATUS_OK)
        goto cleanup;
    vdSqrt((*n), h, h);
    status = vmlGetErrStatus();    // h
    if (status != VML_STATUS_OK)
        goto cleanup;

    // Calculation P2x
    status = ippsMulC_64f(a, (x1 - x0) / d, p2x, (*n));    // a(p1x-p0x)/d
    if (status != ippStsNoErr)
        goto cleanup;
    status = ippsAddC_64f_I(x0, p2x, (*n));    // p2x=p0x+a(p1x-p0x)/d
    if (status != ippStsNoErr)
        goto cleanup;

    // Calculation P2y
    status = ippsMulC_64f(a, (y1 - y0) / d, p2y, (*n));    // a(p1y-p0y)/d
    if (status != ippStsNoErr)
        goto cleanup;
    ippsAddC_64f_I(y0, p2y, (*n));    // p2y=p0y+a(p1y-p0y)/d
    if (status != ippStsNoErr)
        goto cleanup;

    // X3
    status = ippsMulC_64f(h, (y1 - y0) / d, output[0], (*n));    // h(y1-y0)/d
    if (status != ippStsNoErr)
        goto cleanup;
    vdSub((*n), p2x, output[0], &output[0][(*n)]);
    status = vmlGetErrStatus();    // x3=x2-h(y1-y0)/d
    if (status != VML_STATUS_OK)
        goto cleanup;
    vdAdd((*n), p2x, output[0], output[0]);
    status = vmlGetErrStatus();    // x3=x2+h(y1-y0)/d
    if (status != VML_STATUS_OK)
        goto cleanup;

    // Y3
    status = ippsMulC_64f(h, (x1 - x0) / d, output[1], (*n));    // h(x1-x0)/d
    if (status != ippStsNoErr)
        goto cleanup;
    vdAdd((*n), p2y, output[1], &output[1][(*n)]);
    status = vmlGetErrStatus();    // y3=y2+h(x1-x0)/d
    if (status != VML_STATUS_OK)
        goto cleanup;
    vdSub((*n), p2y, output[1], output[1]);
    status = vmlGetErrStatus();    // y3=y2-h(x1-x0)/d
    if (status != VML_STATUS_OK)
        goto cleanup;

    // Deallocation
    cblas_dswap((*n), r0, 1, r0_buff, 1);
    cblas_dswap((*n), r1, 1, r1_buff, 1);

cleanup:
    mkl_free(a);
    mkl_free(h);
    mkl_free(p2x);
    mkl_free(p2y);
    mkl_free(r0_buff);
    mkl_free(r1_buff);
    if (status != 0)
    {    // If error triggered
        mkl_free(output[0]);
        mkl_free(output[1]);
        mkl_free(output);
        *n = 0;
    }

    *n += *n;
    return output;
}