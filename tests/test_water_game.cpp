#include "../source/include/water_game.h"
#include "check.h"

#include <cmath>

namespace {

constexpr float kStep = 1.0f / 120.0f;

void run(WaterGame& game, float seconds) {
    for (float t = 0; t < seconds; t += kStep) {
        game.step(kStep);
    }
}

}  // namespace

// 1. A ring left alone sinks slowly and steadily, and rests on the floor
//
// Slowly is the point: in air it would be falling at 1000 a second after a
// second, in this water it sinks at a steady 200.
void test_a_ring_sinks_slowly_to_the_floor() {
    // Given
    WaterGame game(400, 800);
    int ring = game.add_ring({100, 700}, 20, 12);

    // When: a second, and another
    run(game, 1);
    float after_one = game.ring_position(ring).y;
    run(game, 1);
    float after_two = game.ring_position(ring).y;

    // Then: the first second is spent getting up to speed, and covers far
    // less than the 500 a fall through air would; after that it holds 200
    CHECK_TRUE("sinking", after_one < 700);
    CHECK_TRUE("far less than a fall through air", 700 - after_one < 250);
    CHECK_NEAR("200 in the next second, and no faster", after_one - after_two, 200, 5);

    // And in the end, on the floor
    run(game, 3);
    CHECK_NEAR("resting on the floor", game.ring_position(ring).y, 20, 1);
}

// 2. A pump lifts the ring over its nozzle and leaves one beside it alone
void test_a_pump_lifts_what_is_over_it() {
    // Given: a jet, a ring on the floor over it and one well to the side
    WaterGame game(400, 800);
    int jet = game.add_jet({100, 0});
    int over = game.add_ring({100, 20}, 20, 12);
    int aside = game.add_ring({300, 20}, 20, 12);

    // When: one press, watched for half a second
    game.pump(jet);
    float highest = 0;
    for (float t = 0; t < 0.5f; t += kStep) {
        game.step(kStep);
        highest = std::fmax(highest, game.ring_position(over).y);
    }

    // Then
    CHECK_TRUE("the one over it rose", highest > 80);
    CHECK_NEAR("the one beside it did not", game.ring_position(aside).y, 20, 0.5);
}

// 3. A press is a burst: it dies away by itself and the ring sinks back
void test_a_burst_dies_away() {
    // Given
    WaterGame game(400, 800);
    int jet = game.add_jet({100, 0});
    int ring = game.add_ring({100, 20}, 20, 12);

    // When: one press, then nothing
    game.pump(jet);
    CHECK_NEAR("full on at the press", game.jet_strength(jet), 1, 0.001);
    run(game, 2);

    // Then
    CHECK_EQ("the jet has stopped", game.jet_strength(jet), 0.0f);
    run(game, 3);
    CHECK_NEAR("the ring is back on the floor", game.ring_position(ring).y, 20, 1);
}

// 4. A ring that comes down over a peg's tip is caught and slides down it
void test_a_ring_over_a_peg_is_caught() {
    // Given: a peg standing from 100 to 300, a ring above its tip
    WaterGame game(400, 800);
    int peg = game.add_peg({200, 300}, 200);
    int ring = game.add_ring({201, 600}, 20, 12);

    // When
    run(game, 4);

    // Then: on the peg, centred on it, at the bottom
    CHECK_EQ("on the peg", game.ring_on_peg(ring), peg);
    CHECK_EQ("counted", game.rings_on(peg), 1);
    CHECK_NEAR("centred on the peg", game.ring_position(ring).x, 200, 0.001);
    CHECK_NEAR("slid to the bottom", game.ring_position(ring).y, 103, 0.5);
}

// 5. A ring that comes down beside a peg is not caught
void test_a_ring_beside_a_peg_is_not_caught() {
    // Given: the same peg, a ring coming down well off to the side
    WaterGame game(400, 800);
    int peg = game.add_peg({200, 300}, 200);
    int ring = game.add_ring({260, 600}, 20, 12);

    // When
    run(game, 4);

    // Then
    CHECK_EQ("not on a peg", game.ring_on_peg(ring), -1);
    CHECK_EQ("nothing counted", game.rings_on(peg), 0);
    CHECK_NEAR("on the floor", game.ring_position(ring).y, 20, 1);
}

// 6. A ring that lands on the tip with the rim, not the hole, is not caught
//
// Off by more than the hole but less than the ring, it sits on the peg for a
// moment and then slips off the side.
void test_a_ring_landing_on_its_rim_slips_off() {
    // Given: a ring whose hole cannot reach the tip, but whose rim can
    WaterGame game(400, 800);
    int peg = game.add_peg({200, 300}, 200);
    int ring = game.add_ring({215, 600}, 20, 12);

    // When
    run(game, 5);

    // Then
    CHECK_EQ("not caught by the rim", game.ring_on_peg(ring), -1);
    CHECK_EQ("nothing counted", game.rings_on(peg), 0);
}

// 7. Rings on one peg stack, each resting on the one below
void test_rings_on_a_peg_stack() {
    // Given: two rings, one above the other, over the same peg
    WaterGame game(400, 800);
    int peg = game.add_peg({200, 300}, 200);
    int first = game.add_ring({200, 500}, 20, 12);
    int second = game.add_ring({200, 650}, 20, 12);

    // When
    run(game, 6);

    // Then
    CHECK_EQ("both on the peg", game.rings_on(peg), 2);
    CHECK_EQ("first caught", game.ring_on_peg(first), peg);
    CHECK_EQ("second caught", game.ring_on_peg(second), peg);
    CHECK_NEAR("second rests one ring above the first",
               game.ring_position(second).y - game.ring_position(first).y, 6, 0.5);
}

// 8. Once on a peg, a ring stays there however hard it is pumped
void test_a_caught_ring_stays_caught() {
    // Given: a ring caught on a peg that stands over a jet
    WaterGame game(400, 800);
    int jet = game.add_jet({200, 0});
    int peg = game.add_peg({200, 300}, 200);
    int ring = game.add_ring({200, 600}, 20, 12);
    run(game, 4);
    float settled = game.ring_position(ring).y;

    // When: pumped again and again
    for (int i = 0; i < 10; i++) {
        game.pump(jet);
        run(game, 0.2f);
    }

    // Then
    CHECK_EQ("still on the peg", game.ring_on_peg(ring), peg);
    CHECK_NEAR("not moved", game.ring_position(ring).y, settled, 0.001);
}

// 9. Tilted, the rings sink the new way down, just as fast
void test_tilting_changes_which_way_rings_sink() {
    // Given: down to the right, given at a strength the game should ignore
    WaterGame game(400, 800);
    int ring = game.add_ring({100, 400}, 20, 12);
    game.set_down({50, 0});

    // When
    run(game, 1);

    // Then: drifting right at the usual speed, not falling
    CHECK_NEAR("sank about 200 to the right", game.ring_position(ring).x - 100, 200, 40);
    CHECK_NEAR("no lower than it was", game.ring_position(ring).y, 400, 0.5);
}

// 10. A device lying flat says no direction, and the last one is kept
void test_no_direction_is_ignored() {
    // Given
    WaterGame game(400, 800);
    int ring = game.add_ring({100, 400}, 20, 12);

    // When: told nothing in particular
    game.set_down({0, 0});
    run(game, 1);

    // Then: still sinking straight down, not frozen or lost
    Vec2 at = game.ring_position(ring);
    CHECK_TRUE("a number, not NaN", std::isfinite(at.x) && std::isfinite(at.y));
    CHECK_TRUE("still sinking", at.y < 300);
}

// 11. However hard it is pumped, every ring stays inside the case
void test_rings_stay_inside_the_case() {
    // Given: a small case, both jets, a handful of rings
    WaterGame game(300, 400);
    int left = game.add_jet({80, 0});
    int right = game.add_jet({220, 0});
    for (int i = 0; i < 8; i++) {
        game.add_ring({40.0f + i * 30, 30.0f + (i % 3) * 40}, 15, 9);
    }

    // When: hammered for five seconds
    for (int i = 0; i < 50; i++) {
        game.pump(i % 2 ? left : right);
        run(game, 0.1f);
    }

    // Then
    for (int i = 0; i < 8; i++) {
        Vec2 at = game.ring_position(i);
        CHECK_TRUE("inside the left wall", at.x >= 15 - 1);
        CHECK_TRUE("inside the right wall", at.x <= 300 - 15 + 1);
        CHECK_TRUE("above the floor", at.y >= 15 - 1);
        CHECK_TRUE("below the top", at.y <= 400 - 15 + 1);
    }
}

void run_water_game_tests() {
    test_a_ring_sinks_slowly_to_the_floor();
    test_a_pump_lifts_what_is_over_it();
    test_a_burst_dies_away();
    test_a_ring_over_a_peg_is_caught();
    test_a_ring_beside_a_peg_is_not_caught();
    test_a_ring_landing_on_its_rim_slips_off();
    test_rings_on_a_peg_stack();
    test_a_caught_ring_stays_caught();
    test_tilting_changes_which_way_rings_sink();
    test_no_direction_is_ignored();
    test_rings_stay_inside_the_case();
}
