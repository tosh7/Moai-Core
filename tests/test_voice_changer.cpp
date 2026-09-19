#include "../source/include/fft.h"
#include "../source/include/voice_changer.h"
#include "check.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <string>
#include <vector>

namespace {

constexpr int kRate = 48000;
constexpr int kWindow = 1024;
constexpr float kTau = 2 * std::numbers::pi_v<float>;

std::vector<float> sine(float hz, int length) {
    std::vector<float> out(length);
    for (int t = 0; t < length; t++) {
        out[t] = std::sin(kTau * hz * t / kRate);
    }
    return out;
}

// Pushes a whole signal through in buffers of the given size, the way a
// host would, and returns everything that came out.
std::vector<float> run(VoiceChanger& changer, const std::vector<float>& in, int chunk) {
    std::vector<float> out(in.size());
    for (int at = 0; at < static_cast<int>(in.size()); at += chunk) {
        int n = std::min(chunk, static_cast<int>(in.size()) - at);
        changer.process(in.data() + at, out.data() + at, n);
    }
    return out;
}

// The largest gap between the output and the input delayed by one window,
// looked at only after the start, where fewer than four frames overlap and
// the output is still ramping up.
float worst_error(const std::vector<float>& in, const std::vector<float>& out) {
    float worst = 0;
    for (int t = 2 * kWindow; t < static_cast<int>(out.size()); t++) {
        worst = std::max(worst, std::abs(out[t] - in[t - kWindow]));
    }
    return worst;
}

// One frame bin, in Hz. The crude shift can only land on these.
constexpr float kBinHz = static_cast<float>(kRate) / kWindow;

// Energy within width Hz of a frequency, from a longer look at the output
// than one frame, taken after the warm-up.
float energy_near(const std::vector<float>& x, float hz, float width) {
    const int n = 8192;
    std::vector<std::complex<float>> f(n);
    for (int k = 0; k < n; k++) {
        f[k] = x[8192 + k];
    }
    fft(f.data(), n);
    float total = 0;
    for (int k = 1; k < n / 2; k++) {
        float fk = k * static_cast<float>(kRate) / n;
        if (std::abs(fk - hz) <= width) {
            total += std::norm(f[k]);
        }
    }
    return total;
}

// The loudest frequency in the output, from the same long look.
float peak_hz(const std::vector<float>& x) {
    const int n = 8192;
    std::vector<std::complex<float>> f(n);
    for (int k = 0; k < n; k++) {
        f[k] = x[8192 + k];
    }
    fft(f.data(), n);
    int best = 1;
    for (int k = 2; k < n / 2; k++) {
        if (std::abs(f[k]) > std::abs(f[best])) {
            best = k;
        }
    }
    return best * static_cast<float>(kRate) / n;
}

// How finely the long look can place a peak.
constexpr float kLookHz = static_cast<float>(kRate) / 8192;

// Shifts a tone and reports where it went: the peak on the target, and
// whatever leaks back to the source well below it.
void check_shift(float from, float semitones, const char* label) {
    VoiceChanger changer(kRate, kWindow);
    changer.set_pitch(semitones);
    std::vector<float> in = sine(from, kRate);
    std::vector<float> out = run(changer, in, kWindow);

    float to = from * std::exp2(semitones / 12);
    CHECK_NEAR(std::string(label) + ": peak on the target", peak_hz(out), to, kLookHz);

    float left_behind = energy_near(out, from, kBinHz / 2);
    float arrived = energy_near(out, to, 2 * kBinHz);
    CHECK_TRUE(std::string(label) + ": source at least 30 dB down",
               left_behind * 1000 < arrived);
}

}  // namespace

// 1. With no shift, the voice comes back as it went in, one window later
void test_no_shift_returns_the_input() {
    // GIVEN
    VoiceChanger changer(kRate, kWindow);
    changer.set_pitch(0);
    std::vector<float> in = sine(440, kRate);

    // WHEN
    std::vector<float> out = run(changer, in, kWindow);

    // THEN
    CHECK_NEAR("input comes back", worst_error(in, out), 0, 1e-3);
}

// 2. The delay is exactly one window: that many zeros, then the signal
void test_output_runs_one_window_behind() {
    // GIVEN
    VoiceChanger changer(kRate, kWindow);
    std::vector<float> in = sine(440, kRate);

    // WHEN
    std::vector<float> out = run(changer, in, kWindow);

    // THEN
    bool quiet = std::all_of(out.begin(), out.begin() + kWindow,
                             [](float x) { return x == 0; });
    CHECK_TRUE("nothing before a window has passed", quiet);
    bool loud = std::any_of(out.begin() + kWindow, out.begin() + 2 * kWindow,
                            [](float x) { return std::abs(x) > 0.1f; });
    CHECK_TRUE("something within the next", loud);
}

// 3. Silence gives silence
void test_silence_gives_silence() {
    // GIVEN
    VoiceChanger changer(kRate, kWindow);
    std::vector<float> in(kRate, 0.0f);

    // WHEN
    std::vector<float> out = run(changer, in, kWindow);

    // THEN
    bool silent = std::all_of(out.begin(), out.end(), [](float x) { return x == 0; });
    CHECK_TRUE("silent throughout", silent);
}

// 4. Buffers of a length that never lines up with the window leave no seam
//
// A host hands over whatever size its audio unit likes. 333 divides
// neither the window nor the hop, so every frame straddles a boundary.
void test_awkward_buffers_leave_no_seam() {
    // GIVEN
    VoiceChanger changer(kRate, kWindow);
    std::vector<float> in = sine(440, kRate);

    // WHEN
    std::vector<float> out = run(changer, in, 333);

    // THEN
    CHECK_NEAR("no seam at any boundary", worst_error(in, out), 0, 1e-3);
}

// 5. A buffer larger than the window is handled in one call
void test_one_large_buffer() {
    // GIVEN
    VoiceChanger changer(kRate, kWindow);
    std::vector<float> in = sine(440, kRate);

    // WHEN
    std::vector<float> out = run(changer, in, kRate);

    // THEN
    CHECK_NEAR("whole second in one go", worst_error(in, out), 0, 1e-3);
}

// 6. A tone on a bin lands on the target: up an octave, down one, up a fifth
void test_shift_moves_a_tone_on_a_bin() {
    const float on_bin = 10 * kBinHz;
    check_shift(on_bin, 12, "octave up");
    check_shift(on_bin, -12, "octave down");
    check_shift(on_bin, 7, "fifth up");
}

// 7. A tone between bins lands on the target too
//
// This is what the phase vocoder is for. Moving magnitudes alone puts a
// tone only where a bin is; 440 Hz sits between two, and doubling it by
// bins gives 843 or 937, not 880. Following the phase finds the true
// frequency between the bins and moves that.
void test_shift_moves_a_tone_between_bins() {
    check_shift(440, 12, "440 up an octave");
    check_shift(440, -12, "440 down an octave");
    check_shift(440, 7, "440 up a fifth");
}

// 8. Shifted output stays real and bounded — no NaN, nothing blowing up
void test_shift_stays_sane() {
    // GIVEN
    VoiceChanger changer(kRate, kWindow);
    changer.set_pitch(5);
    std::vector<float> in = sine(440, kRate);

    // WHEN
    std::vector<float> out = run(changer, in, 333);

    // THEN
    bool sane = std::all_of(out.begin(), out.end(),
                            [](float x) { return std::isfinite(x) && std::abs(x) <= 2; });
    CHECK_TRUE("finite and within bounds", sane);
}

void run_voice_changer_tests() {
    test_no_shift_returns_the_input();
    test_output_runs_one_window_behind();
    test_silence_gives_silence();
    test_awkward_buffers_leave_no_seam();
    test_one_large_buffer();
    test_shift_moves_a_tone_on_a_bin();
    test_shift_moves_a_tone_between_bins();
    test_shift_stays_sane();
}
