#include "voice_changer.h"
#include "fft.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

// Frames overlap by three quarters. With a Hann window on the way in and
// again on the way out, the overlapped windows sum to a constant 1.5, so
// dividing by that gives the input back when nothing is done in between.
constexpr int kOverlap = 4;
constexpr float kWindowSum = 1.5f;

static float hann(int k, int n) {
    return 0.5f - 0.5f * std::cos(2 * std::numbers::pi_v<float> * k / n);
}

constexpr float kTau = 2 * std::numbers::pi_v<float>;

// Brings an angle back into (-pi, pi].
static float wrap(float phase) {
    return phase - kTau * std::round(phase / kTau);
}

VoiceChanger::VoiceChanger(int sample_rate, int window) {
    this->sample_rate = sample_rate;

    int size = 1;
    while (size < std::max(window, 64)) {
        size *= 2;
    }
    this->window = size;

    semitones = 0;

    pending.reserve(this->window);
    overlap.assign(this->window, 0.0f);
    last_phase.assign(this->window / 2 + 1, 0.0f);
    phase_sum.assign(this->window / 2 + 1, 0.0f);
 }

void VoiceChanger::set_pitch(float semitones) {
    this->semitones = semitones;
}

void VoiceChanger::process(const float* in, float* out, int count) {
    int hop = window / kOverlap;

    for (int i = 0; i < count; i++) {
        // One out for one in. Nothing is ready until the first frame fills,
        // which is the window of delay the header promises.
        if (ready.empty()) {
            out[i] = 0;
        } else {
            out[i] = ready.front();
            ready.pop_front();
        }

        pending.push_back(in[i]);
        if (static_cast<int>(pending.size()) < window) {
            continue;
        }

        // A frame is full. Window it, go to frequency and back.
        std::vector<std::complex<float>> frame(window);
        for (int k = 0; k < window; k++) {
            frame[k] = pending[k] * hann(k, window);
        }
        fft(frame.data(), window);
        if (semitones != 0) {
            shift(frame.data(), std::exp2(semitones / 12.0f));
        }
        ifft(frame.data(), window);

        // Window again and add onto what earlier frames left here.
        for (int k = 0; k < window; k++) {
            overlap[k] += frame[k].real() * hann(k, window) / kWindowSum;
        }

        // The first hop has now been touched by every frame that will
        // touch it. Send it on, slide the rest down, open up the tail.
        for (int k = 0; k < hop; k++) {
            ready.push_back(overlap[k]);
        }
        for (int k = 0; k < window - hop; k++) {
            overlap[k] = overlap[k + hop];
        }
        for (int k = window - hop; k < window; k++) {
            overlap[k] = 0;
        }

        // Drop one hop of input; the rest overlaps into the next frame.
        pending.erase(pending.begin(), pending.begin() + hop);
    }
}
// A phase vocoder. Moving a bin's magnitude is easy; what makes the result
// a clean tone rather than a robotic one is giving the moved bin a phase
// that advances at the right rate, frame after frame.
void VoiceChanger::shift(std::complex<float>* frame, float ratio) {
    int half = window / 2;
    int hop = window / kOverlap;

    // Analysis. A component that sits between two bins turns a little
    // faster or slower per hop than either bin's own rate. The difference
    // between the phase advance seen and the advance the bin would have on
    // its own, wrapped, is that excess; adding it back gives the true rate.
    std::vector<float> magnitude(half + 1);
    std::vector<float> advance(half + 1);
    for (int k = 0; k <= half; k++) {
        float phase = std::arg(frame[k]);
        float expected = kTau * k * hop / window;
        float seen = phase - last_phase[k];
        advance[k] = expected + wrap(seen - expected);
        last_phase[k] = phase;
        magnitude[k] = std::abs(frame[k]);
    }

    // Synthesis. Each bin goes to ratio times its frequency, and the bin it
    // lands on is run forward at ratio times the true rate. Where more than
    // one lands on the same bin, the loudest sets the pace.
    std::vector<float> moved_magnitude(half + 1, 0.0f);
    std::vector<float> moved_advance(half + 1, 0.0f);
    for (int k = 0; k <= half; k++) {
        int target = static_cast<int>(std::lround(k * ratio));
        if (target > half) {
            continue;
        }
        if (magnitude[k] > moved_magnitude[target]) {
            moved_magnitude[target] = magnitude[k];
            moved_advance[target] = advance[k] * ratio;
        }
    }
    for (int t = 0; t <= half; t++) {
        phase_sum[t] = wrap(phase_sum[t] + moved_advance[t]);
        frame[t] = std::polar(moved_magnitude[t], phase_sum[t]);
    }

    // The upper half mirrors the lower, so the inverse comes out real.
    for (int k = 1; k < half; k++) {
        frame[window - k] = std::conj(frame[k]);
    }
}
