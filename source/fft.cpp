#include "fft.h"

#include <cmath>
#include <numbers>
#include <utility>

static void transform(std::complex<float>* data, int n, float direction) {
    if (n < 2) {
        return;
    }

    int bits = 0;
    while ((1 << bits) < n) {
        ++bits;
    }
    for (int i = 0; i < n; i++) {
        int j = 0;

        for (int b = 0; b < bits; b++) {
            if (i & (1 << b)) {
                j |= 1 << (bits - 1 - b);
            }
        }

        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }

    for (int len = 2; len <= n; len *= 2) {
        int half = len / 2;
        float angle = direction * 2 * std::numbers::pi_v<float> / len;

        for (int start = 0; start < n; start += len) {
            for (int i = 0; i < half; i++) {
                std::complex<float> w = std::polar(1.0f, angle * i);
                std::complex<float> u = data[start + i];
                std::complex<float> v = data[start + i + half] * w;
                data[start + i] = u + v;
                data[start + i + half] = u - v;
            }
        }
    }
}

void fft(std::complex<float>* data, int n) {
    transform(data, n, -1.0f);
}

void ifft(std::complex<float>* data, int n) {
    transform(data, n, 1.0f);
    for (int i = 0; i < n; i++) {
        data[i] /= static_cast<float>(n);
    }
}