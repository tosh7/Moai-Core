#ifndef WATER_GAME_H
#define WATER_GAME_H

#include "world.h"
#include <vector>

// The water ring toss: rings drifting in a sealed tank of water, two pumps
// in the floor that send them up, and pegs to land them on. Built on World,
// which does the water; this adds the pumps, the pegs and the score.
class WaterGame {
    public:
        // The tank, floor at y = 0 and y pointing up, as World.
        WaterGame(float width, float height);

        // radius is the ring's outside, which is what it bumps with; hole is
        // the inside, which is what a peg has to pass through.
        int add_ring(Vec2 position, float radius, float hole);

        // A peg stands straight up from tip.y - length to its tip.
        int add_peg(Vec2 tip, float length);

        // A pump in the floor at nozzle, sending water straight up.
        int add_jet(Vec2 nozzle);

        // One press: a burst of water that dies away on its own. Pressing
        // again restarts it; holding does no more than pressing, as with the
        // toy.
        void pump(int jet);

        // Which way is down, from tilting the device. Only the direction is
        // used; how hard the rings sink is the game's to decide. Straight
        // down until told otherwise.
        void set_down(Vec2 direction);

        void step(float dt);

        Vec2 ring_position(int ring) const;

        // The peg a ring is on, or -1 while it is free. Once on, it stays on.
        int ring_on_peg(int ring) const;

        // How many rings a peg holds. The score.
        int rings_on(int peg) const;

        // 0 to 1, for drawing the bubbles.
        float jet_strength(int jet) const;

    private:
        struct Ring {
            int body;       // in world
            float hole;
            int peg;        // -1 while free
        };

        struct Peg {
            int obstacle;   // in world
            Vec2 tip;
            float length;
            int count;      // rings on it so far
        };

        struct Jet {
            int flow;       // in world, which also holds its strength
        };

        World world;
        std::vector<Ring> rings;
        std::vector<Peg> pegs;
        std::vector<Jet> jets;
};

#endif
