#include "../source/include/fft.h"
#include "check.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <vector>

namespace {

constexpr float kTau = 2 * std::numbers::pi_v<float>;

// The same arbitrary signal every run. A formula rather than a random
// source, so a failure can be reproduced.
std::vector<std::complex<float>> arbitrary_signal(int n) {
    std::vector<std::complex<float>> data(n);
    for (int t = 0; t < n; t++) {
        data[t] = std::complex<float>(std::sin(t * 0.37f), std::cos(t * 1.13f));
    }
    return data;
}

}  // namespace

// 1. An impulse gives a flat spectrum
void test_impulse_gives_flat_spectrum() {
    // GIVEN
    std::vector<std::complex<float>> data(8);
    data[0] = 1;

    // WHEN
    fft(data.data(), 8);

    // THEN
    for (int k = 0; k < 8; k++) {
        CHECK_NEAR("bin " + std::to_string(k), std::abs(data[k] - std::complex<float>(1, 0)), 0, 1e-5);
    }
}

// 2. A constant goes entirely into bin 0
void test_constant_goes_into_bin_zero() {
    // GIVEN
    std::vector<std::complex<float>> data(16, std::complex<float>(1, 0));

    // WHEN
    fft(data.data(), 16);

    // THEN: bin 0 holds the sum, and nothing else is lit
    CHECK_NEAR("bin 0", std::abs(data[0] - std::complex<float>(16, 0)), 0, 1e-4);
    for (int k = 1; k < 16; k++) {
        CHECK_NEAR("bin " + std::to_string(k), std::abs(data[k]), 0, 1e-4);
    }
}

// 3. A sine lights its own bin and the mirror, and nothing else
void test_sine_lights_one_bin_and_its_mirror() {
    // GIVEN: five cycles across 64 samples
    constexpr int n = 64;
    constexpr int cycles = 5;
    std::vector<std::complex<float>> data(n);
    for (int t = 0; t < n; t++) {
        data[t] = std::cos(kTau * cycles * t / n);
    }

    // WHEN
    fft(data.data(), n);

    // THEN: a real input splits its energy between bin k and bin n - k, so
    // each of the pair carries n / 2
    for (int k = 0; k < n; k++) {
        float expected = (k == cycles || k == n - cycles) ? n / 2.0f : 0;
        CHECK_NEAR("bin " + std::to_string(k), std::abs(data[k]), expected, 1e-3);
    }
}

// 4. A complex exponential lights only its own bin, so the rotation runs
//    the right way
//
// A real input lights bin k and bin n - k alike, so test 3 would pass with
// the transform running backwards. This input has no mirror.
void test_rotation_runs_the_right_way() {
    // GIVEN: two cycles across 8 samples, turning anticlockwise
    std::vector<std::complex<float>> data(8);
    for (int t = 0; t < 8; t++) {
        data[t] = std::polar(1.0f, kTau * 2 * t / 8);
    }

    // WHEN
    fft(data.data(), 8);

    // THEN
    CHECK_NEAR("bin 2 lit", std::abs(data[2]), 8, 1e-4);
    CHECK_NEAR("bin 6 dark", std::abs(data[6]), 0, 1e-4);
}

// 5. ifft(fft(x)) gives x back
void test_round_trip_returns_the_input() {
    // GIVEN
    constexpr int n = 1024;
    std::vector<std::complex<float>> original = arbitrary_signal(n);
    std::vector<std::complex<float>> data = original;

    // WHEN
    fft(data.data(), n);
    ifft(data.data(), n);

    // THEN: one check on the worst sample, rather than a thousand checks
    float worst = 0;
    for (int t = 0; t < n; t++) {
        worst = std::max(worst, std::abs(data[t] - original[t]));
    }
    CHECK_NEAR("back where it started", worst, 0, 1e-4);
}

// 6. It agrees with the DFT written out as its definition
//
// The other tests each pick an input with a known answer. This one takes an
// arbitrary input and checks every bin against the O(n^2) sum, which catches
// a slip in the bit reversal or the butterflies that a special input might
// not.
void test_agrees_with_naive_dft() {
    // GIVEN: the definition, one sum per bin. k * t is reduced modulo n
    // because the rotation repeats every n, and it keeps the angle small
    // enough for float to be accurate.
    constexpr int n = 64;
    std::vector<std::complex<float>> input = arbitrary_signal(n);
    std::vector<std::complex<float>> expected(n);
    for (int k = 0; k < n; k++) {
        for (int t = 0; t < n; t++) {
            float angle = -kTau * ((k * t) % n) / n;
            expected[k] += input[t] * std::polar(1.0f, angle);
        }
    }

    // WHEN
    std::vector<std::complex<float>> data = input;
    fft(data.data(), n);

    // THEN
    for (int k = 0; k < n; k++) {
        CHECK_NEAR("bin " + std::to_string(k), std::abs(data[k] - expected[k]), 0, 1e-3);
    }
}

void run_fft_tests() {
    test_impulse_gives_flat_spectrum();
    test_constant_goes_into_bin_zero();
    test_sine_lights_one_bin_and_its_mirror();
    test_rotation_runs_the_right_way();
    test_round_trip_returns_the_input();
    test_agrees_with_naive_dft();
}
