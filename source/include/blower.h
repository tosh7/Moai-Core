#ifndef BLOWER_H
#define BLOWER_H

// A party blower: the paper toy that unrolls when blown into and curls back
// up when let go.
class Blower {
    public:
        // length is how far it reaches fully unrolled, in whatever unit the
        // host draws in. Values of zero or below are brought up to a small
        // positive length rather than refused.
        Blower(float length);

        // How hard it is being blown, 0 for not at all through 1 for as hard
        // as it goes. The value is held until the next call, so the host
        // sets it whenever it reads the mic and need not set it every frame.
        // Anything outside 0 through 1 is clamped.
        void set_breath(float strength);

        // Advances by dt seconds. As with World, the core keeps no clock:
        // the host decides how often this is called and by how much.
        void step(float dt);

        // How far it is unrolled right now, 0 through length. What to draw.
        float extension() const;

    private:
        // Fully unrolled. The ceiling on m_extension.
        float length;

        // The last value handed to set_breath. It keeps pushing until the
        // host says otherwise.
        float breath;

        // The two together are the state of a spring: where it is and which
        // way it is going. Position alone cannot tell unrolling from curling
        // back through the same point.
        float m_extension;
        float m_velocity;
};

#endif
