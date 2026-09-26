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

// 11. A jet lifts what is over it and leaves what is beside it alone
void test_a_flow_carries_what_is_in_it() {
    // Given: water, a jet in the floor pointing up, a body over it and one aside
    World world(400, 800);
    world.set_gravity({0, kGravity});
    world.set_drag(5);
    int jet = world.add_flow({200, 0}, {0, 600}, 400, 40);
    int over = world.add_body({200, 20}, 10);
    int aside = world.add_body({300, 20}, 10);

    // When: the jet runs for a second
    world.set_flow(jet, 1);
    run(world, kStepsPerSecond);

    // Then: the one over it has risen, the one beside it has not
    CHECK_TRUE("lifted off the floor", world.position_of(over).y > 100);
    CHECK_NEAR("beside it, still on the floor", world.position_of(aside).y, 10, 1);
}

// 12. A body in a jet rises to where the fading jet balances gravity, and hangs
//
// The jet carries water up at 600 at the nozzle, fading linearly to nothing
// at 400. A body sinks at gravity / drag = 200, so it hangs where the water
// rises at 200: two thirds of the way up.
void test_a_flow_holds_a_body_where_it_balances_gravity() {
    // Given
    World world(400, 800);
    world.set_gravity({0, kGravity});
    world.set_drag(5);
    int jet = world.add_flow({200, 0}, {0, 600}, 400, 40);
    int body = world.add_body({200, 20}, 10);

    // When: long enough to settle
    world.set_flow(jet, 1);
    run(world, 4 * kStepsPerSecond);

    // Then
    CHECK_NEAR("hanging two thirds of the way up", world.position_of(body).y, 400 * 2 / 3.0, 5);
    CHECK_NEAR("and not moving", world.velocity_of(body).y, 0, 2);
}

// 13. Turned off, a jet lets what it held sink back
void test_a_flow_turned_off_lets_go() {
    // Given: a body held up by a jet
    World world(400, 800);
    world.set_gravity({0, kGravity});
    world.set_drag(5);
    int jet = world.add_flow({200, 0}, {0, 600}, 400, 40);
    int body = world.add_body({200, 20}, 10);
    world.set_flow(jet, 1);
    run(world, 3 * kStepsPerSecond);
    CHECK_TRUE("held up first", world.position_of(body).y > 200);

    // When: the jet is turned off
    world.set_flow(jet, 0);
    run(world, 3 * kStepsPerSecond);

    // Then: back on the floor
    CHECK_NEAR("sank back to the floor", world.position_of(body).y, 10, 1);
}

// 14. Half strength lifts less than full
void test_a_weaker_flow_lifts_less() {
    // Given: two tanks alike but for the jet's strength
    World weak(400, 800), strong(400, 800);
    for (World* w : {&weak, &strong}) {
        w->set_gravity({0, kGravity});
        w->set_drag(5);
        w->add_flow({200, 0}, {0, 600}, 400, 40);
        w->add_body({200, 20}, 10);
    }

    // When
    weak.set_flow(0, 0.5f);
    strong.set_flow(0, 1);
    run(weak, 4 * kStepsPerSecond);
    run(strong, 4 * kStepsPerSecond);

    // Then
    CHECK_TRUE("half the jet, lower", weak.position_of(0).y < strong.position_of(0).y);
    CHECK_TRUE("but still lifted", weak.position_of(0).y > 50);
}

// 15. A flow in air is a fan: with no drag there is nothing to carry with
void test_a_flow_needs_drag_to_carry() {
    // Given: the same jet, but no drag
    World world(400, 800);
    world.set_gravity({0, 0});
    world.set_drag(0);
    int jet = world.add_flow({200, 0}, {0, 600}, 400, 40);
    int body = world.add_body({200, 20}, 10);

    // When
    world.set_flow(jet, 1);
    run(world, kStepsPerSecond);

    // Then: drag is the only way a flow reaches a body
    CHECK_NEAR("unmoved without drag", world.position_of(body).y, 20, 0.01);
}

// 16. Beside the column the water is still, not pushed the other way
//
// Test 11 has its bystander on the floor, where a wrong push down would be
// hidden by the wall. This one floats it mid-water with no gravity, so any
// push at all shows.
void test_beside_a_flow_the_medium_is_still() {
    // Given: no gravity, a jet, a body floating well outside its width
    World world(400, 800);
    world.set_gravity({0, 0});
    world.set_drag(5);
    int jet = world.add_flow({200, 0}, {0, 600}, 400, 40);
    int body = world.add_body({300, 200}, 10);

    // When
    world.set_flow(jet, 1);
    run(world, kStepsPerSecond);

    // Then: exactly where it was
    CHECK_NEAR("not moved at all", world.position_of(body).y, 200, 0.01);
}

// 17. A held body is out of the physics: gravity does not move it
void test_a_held_body_ignores_gravity() {
    // Given: a body held in mid-air
    World world(400, 800);
    world.set_gravity({0, kGravity});
    int body = world.add_body({200, 400}, 10);
    world.hold_body(body);

    // When: a second passes
    run(world, kStepsPerSecond);

    // Then: exactly where it was, and not moving
    CHECK_TRUE("held", world.is_held(body));
    CHECK_NEAR("not fallen", world.position_of(body).y, 400, 0.001);
    CHECK_NEAR("not moving", world.velocity_of(body).y, 0, 0.001);
}

// 18. A body dropped onto a held one rests on it, and the held one stays put
//
// This is a ring caught on a peg with the next ring landing on it.
void test_a_body_rests_on_a_held_one() {
    // Given: a held body, and a free one above it
    World world(400, 800);
    world.set_gravity({0, kGravity});
    int held = world.add_body({200, 300}, 20);
    world.hold_body(held);
    int free = world.add_body({200, 500}, 10);

    // When: long enough to land and settle
    run(world, 300);

    // Then: the held one has not given way, the free one sits on top of it
    CHECK_NEAR("held one not pushed down", world.position_of(held).y, 300, 0.001);
    CHECK_NEAR("free one resting on top", world.position_of(free).y, 330, 1);
    CHECK_NEAR("and still", world.velocity_of(free).y, 0, 3);
}

// 19. A held body goes exactly where it is put, and pushes what is in the way
//
// This is a finger dragging a drop into another.
void test_a_held_body_is_put_where_the_host_says() {
    // Given: no gravity, a held body and a free one in its path
    World world(400, 800);
    world.set_gravity({0, 0});
    int finger = world.add_body({100, 200}, 20);
    int other = world.add_body({200, 200}, 20);
    world.hold_body(finger);

    // When: it is moved a little each step, towards and past the other
    for (int i = 0; i < kStepsPerSecond; ++i) {
        world.move_body(finger, {100.0f + i * 2.0f, 200});
        world.step(kStep);
    }

    // Then: the held one is exactly where it was last put, the other pushed ahead
    CHECK_NEAR("held one where it was put", world.position_of(finger).x, 218, 0.001);
    CHECK_TRUE("other pushed clear ahead of it",
               world.position_of(other).x >= world.position_of(finger).x + 40 - 0.5f);
}

// 20. Let go, it sets off at the velocity given and is back in the physics
void test_a_released_body_sets_off_and_falls() {
    // Given: a body held in mid-air
    World world(400, 800);
    world.set_gravity({0, kGravity});
    int body = world.add_body({100, 400}, 10);
    world.hold_body(body);
    run(world, 10);

    // When: let go moving sideways, and given a moment
    world.release_body(body, {300, 0});
    run(world, 10);

    // Then: no longer held, carrying on sideways, and falling
    CHECK_FALSE("no longer held", world.is_held(body));
    CHECK_NEAR("carrying on sideways", world.velocity_of(body).x, 300, 0.001);
    CHECK_TRUE("and falling", world.velocity_of(body).y < 0);
}

// 21. Moving or releasing a body that is not held does nothing
void test_move_and_release_ignore_a_free_body() {
    // Given: a free body, moving
    World world(400, 800);
    world.set_gravity({0, 0});
    int body = world.add_body({100, 400}, 10);
    world.set_velocity(body, {50, 0});

    // When: told to move and to release without having been held
    world.move_body(body, {300, 300});
    world.release_body(body, {-500, 0});

    // Then: neither took
    CHECK_NEAR("not moved", world.position_of(body).x, 100, 0.001);
    CHECK_NEAR("velocity not overwritten", world.velocity_of(body).x, 50, 0.001);
    CHECK_FALSE("never held", world.is_held(body));
}

// 22. A held body is not pushed out of an obstacle or a wall it was put into
void test_a_held_body_is_not_pushed_out() {
    // Given: a shelf, and a body held right inside it; another held past the wall
    World world(400, 800);
    world.set_gravity({0, kGravity});
    world.add_obstacle({200, 300}, {100, 10}, 0);
    int inside = world.add_body({200, 300}, 10);
    int beyond = world.add_body({-50, 400}, 10);
    world.hold_body(inside);
    world.hold_body(beyond);

    // When
    run(world, 10);

    // Then: both exactly where they were put
    CHECK_NEAR("left inside the shelf", world.position_of(inside).y, 300, 0.001);
    CHECK_NEAR("left beyond the wall", world.position_of(beyond).x, -50, 0.001);
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
    test_a_flow_carries_what_is_in_it();
    test_a_flow_holds_a_body_where_it_balances_gravity();
    test_a_flow_turned_off_lets_go();
    test_a_weaker_flow_lifts_less();
    test_a_flow_needs_drag_to_carry();
    test_beside_a_flow_the_medium_is_still();
    test_a_held_body_ignores_gravity();
    test_a_body_rests_on_a_held_one();
    test_a_held_body_is_put_where_the_host_says();
    test_a_released_body_sets_off_and_falls();
    test_move_and_release_ignore_a_free_body();
    test_a_held_body_is_not_pushed_out();
}
