// The 2D physics core the emoji pit runs on.

#include "world.h"

#include <algorithm>
#include <cmath>
#include <iterator>

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

// A tap. Everything within reach is shoved directly away from the point,
// harder the closer it is, so the middle of a pile scatters and the edges
// only shift.
void World::apply_radial_impulse(Vec2 center, float radius, float strength) {
    for (Body& body : bodies) {
        float dx = body.position.x - center.x;
        float dy = body.position.y - center.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > radius || distance == 0) {
            continue;
        }

        float falloff = (radius - distance) / radius;
        body.velocity.x += dx / distance * strength * falloff;
        body.velocity.y += dy / distance * strength * falloff;
    }
}

// Semi-implicit Euler: the new velocity is what moves the body, not the old
// one. Integrating the other way round leaves a stack of bodies shivering
// instead of coming to rest.
void World::step(float dt) {
    for (Body& body : bodies) {
        body.velocity.x += gravity.x * dt;
        body.velocity.y += gravity.y * dt;

        body.position.x += body.velocity.x * dt;
        body.position.y += body.velocity.y * dt;
    }

    // Resolving once is enough for two bodies meeting in open space, but not
    // for a pile. A column dropped together falls at one speed, so no pair is
    // closing on any other and no contact has anything to correct; only the
    // floor knows better, and its grip reaches the top a fraction at a time.
    // Ninety bodies at this many passes cost a fiftieth of a frame, so the
    // count is set by how still the pile has to look, not by the budget.
    constexpr int passes = 32;
    for (int pass = 0; pass < passes; ++pass) {
        // Every pair once. The inner loop starts after the outer one so a pair
        // is never pushed apart twice, and so a body never meets itself.
        for (auto a = bodies.begin(); a != bodies.end(); ++a) {
            for (auto b = std::next(a); b != bodies.end(); ++b) {
                float dx = b->position.x - a->position.x;
                float dy = b->position.y - a->position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                float overlap = a->radius + b->radius - distance;

                if (overlap <= 0 || distance == 0) {
                    continue;
                }

                // Unit vector pointing from a to b.
                float nx = dx / distance;
                float ny = dy / distance;

                // Share the overlap out, half each, so neither is favoured.
                // Only most of it, though: separating a resting pair cleanly
                // would hide them from the next pass, and the pass after that
                // is where their speeds finish cancelling out.
                constexpr float recovery = 0.8f;
                float push = overlap * recovery * 0.5f;
                a->position.x -= nx * push;
                a->position.y -= ny * push;
                b->position.x += nx * push;
                b->position.y += ny * push;

                float closing = (b->velocity.x - a->velocity.x) * nx
                              + (b->velocity.y - a->velocity.y) * ny;

                // Already drawing apart. Reflecting now would pull them back
                // together and leave a resting pair humming.
                if (closing > 0) {
                    continue;
                }

                // A body that has merely sagged onto its neighbour under
                // gravity should not bounce off it; only a real impact does.
                constexpr float bounce_threshold = 60.0f;
                float restitution = -closing > bounce_threshold ? 0.5f : 0.0f;
                float impulse = -(1 + restitution) * closing * 0.5f;

                a->velocity.x -= impulse * nx;
                a->velocity.y -= impulse * ny;
                b->velocity.x += impulse * nx;
                b->velocity.y += impulse * ny;
            }
        }

        // Obstacles are immovable, so unlike a pair of bodies the correction
        // falls entirely on the body: it takes the whole overlap, and the whole
        // of the impulse.
        for (Body& body : bodies) {
            for (const Obstacle& obstacle : obstacles) {
                // Work in the obstacle's own frame, where it is axis aligned
                // and the nearest point on it is a matter of clamping.
                float c = std::cos(obstacle.angle);
                float s = std::sin(obstacle.angle);
                float dx = body.position.x - obstacle.center.x;
                float dy = body.position.y - obstacle.center.y;
                float local_x =  dx * c + dy * s;
                float local_y = -dx * s + dy * c;

                float closest_x = std::clamp(local_x, -obstacle.half_size.x,
                                             obstacle.half_size.x);
                float closest_y = std::clamp(local_y, -obstacle.half_size.y,
                                             obstacle.half_size.y);

                float offset_x = local_x - closest_x;
                float offset_y = local_y - closest_y;
                float distance = std::sqrt(offset_x * offset_x + offset_y * offset_y);

                float normal_x = 0;
                float normal_y = 0;
                float overlap = 0;

                if (distance > 0) {
                    if (distance >= body.radius) {
                        continue;
                    }
                    normal_x = offset_x / distance;
                    normal_y = offset_y / distance;
                    overlap = body.radius - distance;
                } else {
                    // Dead centre inside the rectangle, which a fast body can
                    // reach in a single step. Leave by the nearest face.
                    float out_x = obstacle.half_size.x - std::abs(local_x);
                    float out_y = obstacle.half_size.y - std::abs(local_y);

                    if (out_x < out_y) {
                        normal_x = local_x < 0 ? -1.0f : 1.0f;
                        overlap = out_x + body.radius;
                    } else {
                        normal_y = local_y < 0 ? -1.0f : 1.0f;
                        overlap = out_y + body.radius;
                    }
                }

                // Back into the world's frame.
                float world_nx = normal_x * c - normal_y * s;
                float world_ny = normal_x * s + normal_y * c;

                body.position.x += world_nx * overlap;
                body.position.y += world_ny * overlap;

                float closing = body.velocity.x * world_nx
                              + body.velocity.y * world_ny;

                // Only the part of the motion heading into the obstacle is
                // taken away; whatever it was doing along the face it keeps.
                if (closing < 0) {
                    body.velocity.x -= world_nx * closing;
                    body.velocity.y -= world_ny * closing;
                }
            }
        }

        // The walls have the last word, so a body pushed out of the world by
        // its neighbours is put back before anything reads its position.
        for (Body& body : bodies) {
            if (body.position.x < body.radius) {
                body.position.x = body.radius;
                body.velocity.x = 0;
            } else if (body.position.x > width - body.radius) {
                body.position.x = width - body.radius;
                body.velocity.x = 0;
            }

            if (body.position.y < body.radius) {
                body.position.y = body.radius;
                body.velocity.y = 0;
            } else if (body.position.y > height - body.radius) {
                body.position.y = height - body.radius;
                body.velocity.y = 0;
            }
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

int World::add_obstacle(Vec2 center, Vec2 half_size, float angle) {
    obstacles.push_back({center, half_size, angle, center, angle});
    return static_cast<int>(obstacles.size()) - 1;
}

void World::move_obstacle(int index, Vec2 center, float angle) {
    obstacles[index].center = center;
    obstacles[index].angle = angle;
}

int World::obstacle_count() const {
    return static_cast<int>(obstacles.size());
}
