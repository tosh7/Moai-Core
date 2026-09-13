#ifndef FFT_H
#define FFT_H

#include <complex>

// Both transform data in place: n complex values go in, n come out in the
// same array. n must be a power of two.

// Time to frequency. data[k] becomes the amplitude and phase of the k-th
// bin, whose frequency is k * (sample rate / n).
void fft(std::complex<float>* data, int n);

// Frequency back to time. Includes the 1/n scaling, so ifft(fft(x)) is x.
void ifft(std::complex<float>* data, int n);

#endif
