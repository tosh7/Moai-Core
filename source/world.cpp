// The 2D physics core the emoji pit runs on.
//
// Bodies fall and land. They do not yet notice each other or the side walls,
// so the collision half of tests/test_world.cpp stays commented out.

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

// Semi-implicit Euler: the new velocity is what moves the body, not the old
// one. Integrating the other way round leaves a stack of bodies shivering
// instead of coming to rest.
void World::step(float dt) {
    for (Body& body : bodies) {
        body.velocity.x += gravity.x * dt;
        body.velocity.y += gravity.y * dt;

        body.position.x += body.velocity.x * dt;
        body.position.y += body.velocity.y * dt;

        // The floor holds the body up and takes its downward speed away.
        if (body.position.y < body.radius) {
            body.position.y = body.radius;
            body.velocity.y = 0;
        }
    }
}

int World::body_count() const {
    return static_cast<int>(bodies.size());
}

Vec2 World::position_of(int index) const {
    return bodies[index].position;
}

Vec2 World::velocity_of(int index) const {
    return bodies[index].velocity;
}
