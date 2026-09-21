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

        // How quickly a body loses speed to what it moves through, per
        // second: 0 is air and changes nothing, a few is water. Bodies only;
        // obstacles are not slowed.
        void set_drag(float drag);
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

        // A region where the medium itself is moving — a jet, a fan. A body
        // in it is carried along, since drag acts against the medium's
        // velocity rather than against rest. origin is where it starts;
        // direction is which way the medium moves and how fast, at full
        // strength; it fades to nothing reach away from the origin and
        // width away from its centre line.
        int add_flow(Vec2 origin, Vec2 direction, float reach, float width);

        // 0 is off, 1 is full. Held until set again, so a jet that should
        // die away after a press is the caller's to turn down each step.
        void set_flow(int index, float strength);
        float flow_strength(int index) const;

        // A held body is out of the physics and in the host's hands: gravity,
        // drag and flows leave it alone, and it goes where move_body puts it.
        // It is still there to the others, which bump off it as off a wall.
        // A finger dragging something, a ring caught on a peg.
        void hold_body(int index);
        void move_body(int index, Vec2 position);      // held bodies only; others ignore it

        // Back into the physics, setting off at the velocity given — the
        // finger's speed when it let go, or zero to drop it where it is.
        void release_body(int index, Vec2 velocity);
        bool is_held(int index) const;

    private:
        struct Body {
            Vec2 position;
            Vec2 velocity;
            float radius;
            bool held;
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

        struct Flow {
            Vec2 origin;
            Vec2 direction;
            float reach;
            float width;
            float strength;
        };

        float width;
        float height;
        Vec2 gravity;
        float drag;
        std::vector<Body> bodies;
        std::vector<Obstacle> obstacles;
        std::vector<Flow> flows;

        // The medium's velocity at a point, summed over every flow.
        Vec2 medium_velocity_at(Vec2 position) const;
};

#endif
