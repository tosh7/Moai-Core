#ifndef BLOWER_H
#define BLOWER_H

// A party blower: the paper toy that unrolls when blown into and curls back
// up when let go. Breath pushes it out, the curl pulls it back, and friction
// settles it.
class Blower {
    public:
        // length is how far it reaches fully unrolled. It is the only thing
        // the host has to say about the toy itself.
        Blower(float length);

        // How hard it is being blown, 0 through 1, held until set again. The
        // host reads the mic and hands the level over; the core never hears
        // any audio.
        void set_breath(float strength);

        // Advances by dt. As with World, the host owns the clock.
        void step(float dt);

        // How far it is unrolled, 0 through length. What the host draws.
        float extension() const;

        // How fast that is changing. A tip that is whipping out or snapping
        // back can be drawn bending; a resting one, straight.
        float rate() const;

    private:
        float length;
        float breath;
        float m_extension;
        float m_velocity;
};

#endif
