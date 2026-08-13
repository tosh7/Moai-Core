// The 2D physics core the emoji pit runs on.
//
// Not built yet: add this file and tests/test_world.cpp to test.sh, and
// uncomment run_world_tests() in tests/main.cpp, once step() can satisfy them.

#include "world.h"

World::World(float w, float h) {
    width = w;
    height = h;
    gravity = {0, 0};
}

int World::add_body(Vec2 position, float radius) {
    bodies.push_back({position, {0, 0}, radius});
    return static_cast<int>(bodies.size()) - 1;
}

void World::set_gravity(Vec2 g) {
    gravity = g;
}

void World::set_velocity(int index, Vec2 v) {
    bodies[index].velocity = v;
}

void World::apply_radial_impulse(Vec2 center, float radius, float strength) {}

int World::body_count() const {
    return static_cast<int>(bodies.size());
}

Vec2 World::position_of(int index) const {
    return bodies[index].position;
}

Vec2 World::velocity_of(int index) const {
    return bodies[index].velocity;
}
