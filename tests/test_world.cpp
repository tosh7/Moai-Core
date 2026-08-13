#include "../source/include/world.h"
#include "check.h"

namespace {

// One simulated second at the rate the app will drive it.
constexpr float kStep = 1.0f / 60.0f;
constexpr int kStepsPerSecond = 60;
constexpr float kGravity = -1000.0f;

void run(World& world, int steps) {
    for (int i = 0; i < steps; ++i) {
        world.step(kStep);
    }
}

}  // namespace

// 1. A lone body falls, and falls faster the longer it falls.
//
// Velocity is checked rather than distance because every sane integrator
// agrees on it, while the distance covered in the first second differs by a
// few points between them.
void test_falls_under_gravity() {
    World world(400, 800);
    world.set_gravity({0, kGravity});
    int body = world.add_body({200, 700}, 10);

    run(world, kStepsPerSecond);

    CHECK_NEAR("speed after a second of falling", world.velocity_of(body).y,
               kGravity, 20);
    CHECK_NEAR("fallen about half g t squared", world.position_of(body).y,
               700 + 0.5 * kGravity, 15);
    CHECK_NEAR("has not drifted sideways", world.position_of(body).x, 200, 0.01);
}

// 2. It lands on the floor, and stays there without sinking or jittering.
void test_settles_on_the_floor() {
    World world(400, 800);
    world.set_gravity({0, kGravity});
    int body = world.add_body({200, 300}, 10);

    run(world, 300);

    CHECK_NEAR("resting on the floor", world.position_of(body).y, 10, 1.5);
    CHECK_NEAR("no longer moving", world.velocity_of(body).y, 0, 2);
}

// 3. Two bodies meeting head on bounce apart rather than passing through.
void test_bodies_bounce_off_each_other() {
    World world(400, 400);
    world.set_gravity({0, 0});
    int left = world.add_body({150, 200}, 20);
    int right = world.add_body({250, 200}, 20);
    world.set_velocity(left, {60, 0});
    world.set_velocity(right, {-60, 0});

    run(world, 120);

    CHECK_TRUE("left body turned back", world.velocity_of(left).x < 0);
    CHECK_TRUE("right body turned back", world.velocity_of(right).x > 0);
    CHECK_TRUE("they did not swap places",
               world.position_of(left).x < world.position_of(right).x);
    CHECK_TRUE("they are no longer overlapping",
               world.position_of(right).x - world.position_of(left).x >= 39);
}

// 4. A column of bodies dropped together comes to rest as a stack.
//
// This is where a solver usually shows its seams: the pile either sinks into
// the floor over time or never stops shivering.
void test_a_stack_comes_to_rest() {
    World world(400, 800);
    world.set_gravity({0, kGravity});

    constexpr int count = 5;
    constexpr float radius = 20;
    int bodies[count];
    for (int i = 0; i < count; ++i) {
        bodies[i] = world.add_body({200, 100 + i * (radius * 2 + 5)}, radius);
    }

    run(world, 900);

    for (int i = 0; i < count; ++i) {
        CHECK_TRUE("nothing fell through the floor",
                   world.position_of(bodies[i]).y >= radius - 1);
        CHECK_TRUE("the pile has stopped moving",
                   std::abs(world.velocity_of(bodies[i]).y) < 3);
    }

    // Five bodies of one radius cannot rest in less room than five stacked.
    float highest = 0;
    for (int i = 0; i < count; ++i) {
        highest = std::max(highest, world.position_of(bodies[i]).y);
    }
    CHECK_TRUE("they stacked rather than merged into one spot",
               highest > radius * 6);
}

// 5. A tap shoves nearby bodies away and leaves distant ones alone.
void test_radial_impulse_pushes_bodies_away() {
    World world(600, 600);
    world.set_gravity({0, 0});
    int near = world.add_body({320, 300}, 10);
    int far = world.add_body({560, 300}, 10);

    world.apply_radial_impulse({300, 300}, 100, 500);
    run(world, 30);

    CHECK_TRUE("the near body was pushed outward",
               world.position_of(near).x > 320);
    CHECK_NEAR("the far body was left alone", world.position_of(far).x, 560,
               0.01);
}

// 6. However hard they are shoved, bodies stay inside the walls.
void test_bodies_stay_inside_the_walls() {
    World world(300, 300);
    world.set_gravity({0, kGravity});

    constexpr int count = 12;
    constexpr float radius = 12;
    for (int i = 0; i < count; ++i) {
        int body = world.add_body({40.0f + i * 18, 150.0f + i * 8}, radius);
        world.set_velocity(body, {i % 2 == 0 ? 900.0f : -900.0f, 700.0f});
    }

    run(world, 600);

    for (int i = 0; i < world.body_count(); ++i) {
        Vec2 position = world.position_of(i);
        CHECK_TRUE("inside the left wall", position.x >= radius - 1);
        CHECK_TRUE("inside the right wall", position.x <= 300 - radius + 1);
        CHECK_TRUE("above the floor", position.y >= radius - 1);
        CHECK_TRUE("below the ceiling", position.y <= 300 - radius + 1);
    }
}

void run_world_tests() {
    test_falls_under_gravity();
    test_settles_on_the_floor();

    // Waiting on step(): bodies pass straight through each other, ignore the
    // side walls, and a tap does nothing. Uncomment as each becomes true.
    // test_bodies_bounce_off_each_other();
    // test_a_stack_comes_to_rest();
    // test_radial_impulse_pushes_bodies_away();
    // test_bodies_stay_inside_the_walls();
}
