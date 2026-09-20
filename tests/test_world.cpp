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

// 7. A body dropped onto an obstacle rests on it instead of falling past.
//
// The two heights are chosen far apart on purpose: 330 if the shelf holds it,
// 20 if the shelf is not there at all. A test that cannot tell those apart is
// not testing the shelf.
void test_a_body_lands_on_an_obstacle() {
    // Given: a shelf across the middle of the pit, and a body above it
    World world(400, 800);
    world.set_gravity({0, kGravity});
    world.add_obstacle({200, 300}, {100, 10}, 0);
    int body = world.add_body({200, 600}, 20);

    // When: it is given far longer than the fall needs
    run(world, 300);

    // Then: it is sitting on the shelf — its top, plus the body's radius
    CHECK_NEAR("resting on top of the obstacle", world.position_of(body).y, 330, 2);
    CHECK_NEAR("no longer moving", world.velocity_of(body).y, 0, 3);
    CHECK_TRUE("did not reach the floor", world.position_of(body).y > 100);
}

// 8. In something thick, a body sinks at a steady speed rather than falling
//
// Test 1 shows a body in air a second on still gaining speed. The same body
// in water reaches a speed at which gravity and drag agree, and stays there:
// far slower than the fall, and the same from one second to the next.
void test_drag_settles_a_fall_to_a_steady_speed() {
    // Given: a tall pit of something thick, and a body high up in it
    World world(400, 4000);
    world.set_gravity({0, kGravity});
    world.set_drag(5);
    int body = world.add_body({200, 3900}, 10);

    // When: it has fallen for two seconds, and then for one more
    run(world, 2 * kStepsPerSecond);
    float after_two = world.velocity_of(body).y;
    run(world, kStepsPerSecond);
    float after_three = world.velocity_of(body).y;

    // Then: it is sinking, far slower than a fall, and no faster than before
    CHECK_TRUE("sinking, not rising", after_two < 0);
    CHECK_TRUE("far slower than a fall in air", after_two > kGravity / 2);
    CHECK_NEAR("at a steady speed", after_three, after_two, 0.1);

    // gravity / drag is exactly where the two balance: 1000 / 5 = 200
    CHECK_NEAR("at the speed where gravity and drag agree", after_three, -200, 0.5);
}

// 9. Drag alone brings a moving body to rest
void test_drag_brings_a_body_to_rest() {
    // Given: no gravity, thick medium, a body shoved sideways
    World world(400, 400);
    world.set_gravity({0, 0});
    world.set_drag(5);
    int body = world.add_body({100, 200}, 10);
    world.set_velocity(body, {300, 0});

    // When: a moment passes, then a while
    run(world, 10);
    float soon = world.velocity_of(body).x;
    run(world, 3 * kStepsPerSecond);
    float later = world.velocity_of(body).x;

    // Then: still moving at first, all but stopped later, never reversed
    CHECK_TRUE("still going after a moment", soon > 100);
    CHECK_NEAR("all but stopped later", later, 0, 1);
    CHECK_TRUE("never turned back", later >= 0);
}

// 10. A drag too large for the step stops the body instead of reversing it
void test_excessive_drag_stops_rather_than_reverses() {
    // Given: a drag no step of 1/60 could survive
    World world(400, 400);
    world.set_gravity({0, 0});
    world.set_drag(1000);
    int body = world.add_body({100, 200}, 10);
    world.set_velocity(body, {300, 0});

    // When: one step
    run(world, 1);

    // Then: stopped, not thrown backwards
    CHECK_EQ("stopped dead", world.velocity_of(body).x, 0.0f);
}

void run_world_tests() {
    test_falls_under_gravity();
    test_settles_on_the_floor();
    test_bodies_bounce_off_each_other();
    test_a_stack_comes_to_rest();
    test_radial_impulse_pushes_bodies_away();
    test_bodies_stay_inside_the_walls();
    test_a_body_lands_on_an_obstacle();
    test_drag_settles_a_fall_to_a_steady_speed();
    test_drag_brings_a_body_to_rest();
    test_excessive_drag_stops_rather_than_reverses();
}
