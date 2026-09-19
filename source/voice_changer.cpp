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
        // This is where the pitch will move.
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