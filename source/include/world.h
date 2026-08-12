#ifndef WORLD_H
#define WORLD_H

#include <vector>

struct Vec2 {
    float x;
    float y;
};

class World {
    public:
        World(float width, float height);

        int add_body(Vec2 position, float radius);
        void set_gravity(Vec2 g);
        void set_velocity(int index, Vec2 v);
        void apply_radial_impulse(Vec2 center, float radius, float strength);

        // Advances the world by dt. The caller owns the clock, so a test can
        // run a minute of falling in an instant and the app can slow it down.
        void step(float dt);

        int  body_count() const;
        Vec2 position_of(int index) const;
        Vec2 velocity_of(int index) const;

    private:
        struct Body {
            Vec2 position;
            Vec2 velocity;
            float radius;
        };

        float width;
        float height;
        Vec2 gravity;
        std::vector<Body> bodies;
};

#endif