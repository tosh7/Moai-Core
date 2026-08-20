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

        int add_obstacle(Vec2 center, Vec2 half_size, float angle);
        void move_obstacle(int index, Vec2 center, float angle);
        int obstacle_count() const;

        // An obstacle can be left free to turn, in which case what lands on it
        // spins it and it spins whatever it touches. Radians per second.
        void set_obstacle_spin(int index, float spin);
        float obstacle_angle(int index) const;
        float obstacle_spin(int index) const;

    private:
        struct Body {
            Vec2 position;
            Vec2 velocity;
            float radius;
        };

        struct Obstacle {
            Vec2 center;
            Vec2 half_size;
            float angle;
            // Where it was when the last step began. The difference is how fast it
            // is moving, which is what lets a turning blade fling rather than nudge
            Vec2 previous_center;
            float previous_angle;

            // How fast it is turning, and how hard it is to turn. A body that
            // strikes away from the middle gives it a shove; one that lands on
            // the pivot gives it nothing.
            float spin;
            float inertia;
        };

        float width;
        float height;
        Vec2 gravity;
        std::vector<Body> bodies;
        std::vector<Obstacle> obstacles;
};

#endif