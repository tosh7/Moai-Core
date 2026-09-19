#ifndef VOICE_CHANGER_H
#define VOICE_CHANGER_H

#include <deque>
#include <vector>

// Shifts the pitch of a voice as it streams through. Samples go in, samples
// come out; the mic and the speaker stay with the host.
class VoiceChanger {
    public:
        // sample_rate is the host's, in Hz. window is the FFT size and must
        // be a power of two: larger hears pitch more finely, smaller answers
        // sooner. 1024 at 48 kHz is about 21 ms.
        VoiceChanger(int sample_rate, int window);

        // How far to move the pitch, in semitones: +12 an octave up, -12
        // down, 0 leaves the voice as it is. Held until set again.
        void set_pitch(float semitones);

        // Transforms count samples from in to out. Called with each buffer
        // the host receives; the two may be any length and need not match
        // the window. Whole frames are transformed as they fill, so the
        // output runs one window behind the input.
        void process(const float* in, float* out, int count);

    private:
        int sample_rate;
        int window;
        float semitones;

        // Samples that have arrived but do not yet fill a frame.
        std::vector<float> pending;

        // The tail of the last frame, still to be added to the head of the
        // next. Overlap-add cannot hand a sample out until every frame that
        // touches it has been added in.
        std::vector<float> overlap;

        // Finished samples waiting to go out, one per sample that comes in.
        std::deque<float> ready;
};

#endif
