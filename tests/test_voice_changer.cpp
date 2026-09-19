#include "../source/include/voice_changer.h"
#include "check.h"

#include <algorithm>
#include <cmath>
#include <numbers>
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

void run_voice_changer_tests() {
    test_no_shift_returns_the_input();
    test_output_runs_one_window_behind();
    test_silence_gives_silence();
    test_awkward_buffers_leave_no_seam();
    test_one_large_buffer();
}
