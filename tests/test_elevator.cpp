#include "../source/include/elevator.h"
#include "check.h"

// 1. Starts on floor 1 right after construction
void test_initial_state() {
    Elevator elevator(10, 1);
    CHECK_EQ("initial floor is 1", elevator.current_floor, 1);
}

// 2. After a request, each step moves one floor toward the destination
void test_moves_one_floor_per_step() {
    Elevator elevator(10, 1);
    elevator.request({5, Direction::UP});
    for (int expected = 2; expected <= 5; ++expected) {
        elevator.step();
        CHECK_EQ("moving up to floor " + std::to_string(expected), elevator.current_floor, expected);
    }
}

// 3. Once arrived, the request clears and it moves no further
void test_stops_at_destination() {
    Elevator elevator(10, 1);
    elevator.request({3, Direction::UP});
    for (int i = 0; i < 5; ++i) {
        elevator.step();
    }
    CHECK_EQ("stays at destination", elevator.current_floor, 3);
}

// 4. A call added later but closer is picked up on the way, not doubled back to
void test_serves_calls_in_travel_order() {
    Elevator elevator(10, 1);
    elevator.request({5, Direction::UP});
    elevator.request({3, Direction::UP});
    for (int i = 0; i < 6; ++i) {
        elevator.step();
    }
    // Serving 3 while passing means the run ends at 5 and stays there.
    CHECK_EQ("ends at 5 without doubling back", elevator.current_floor, 5);
}

// 5. Once nothing is left ahead, the car reverses and finishes the other side
void test_reverses_when_nothing_is_ahead() {
    Elevator elevator(10, 1);
    elevator.request({5, Direction::UP});
    for (int i = 0; i < 5; ++i) {
        elevator.step();
    }
    CHECK_EQ("parked at 5 while idle", elevator.current_floor, 5);

    elevator.request({9, Direction::UP});
    elevator.request({4, Direction::UP});
    for (int i = 0; i < 8; ++i) {
        elevator.step();
    }
    // 4 is nearest, so the car dips down first, then reverses and runs up to 9.
    CHECK_EQ("ends at 9 after reversing", elevator.current_floor, 9);
}

// 6. A passenger aboard chooses a floor; the car goes there and stays
void test_takes_a_passenger_to_the_floor_they_chose() {
    Elevator elevator(10, 1);
    elevator.select_floor(5);
    CHECK_TRUE("5 lights up once chosen", elevator.is_selected(5));

    for (int i = 0; i < 6; ++i) {
        elevator.step();
    }
    CHECK_EQ("arrives at 5 and stays", elevator.current_floor, 5);
    CHECK_FALSE("5 goes dark once served", elevator.is_selected(5));
}

// 7. A chosen floor is served whichever way the car happens to be heading
//
// A hall call only boards when it matches the heading. A car call has no
// direction to match: the passenger is already aboard and simply gets off.
void test_lets_a_passenger_off_whichever_way_it_is_heading() {
    Elevator elevator(10, 1);
    elevator.request({8, Direction::UP});
    for (int i = 0; i < 8; ++i) {
        elevator.step();
    }
    CHECK_EQ("parked at 8", elevator.current_floor, 8);

    // A call from 2 sends the car down; 5 is chosen on the way.
    elevator.request({2, Direction::DOWN});
    elevator.select_floor(5);
    for (int i = 0; i < 4; ++i) {
        elevator.step();
    }
    CHECK_EQ("has passed 5 heading down", elevator.current_floor, 4);
    CHECK_FALSE("5 was served on the way down", elevator.is_selected(5));

    // Had 5 been skipped, the car would double back for it after 2.
    for (int i = 0; i < 4; ++i) {
        elevator.step();
    }
    CHECK_EQ("finishes at 2 with nothing left", elevator.current_floor, 2);
}

// 8. A parked car works out which way to go from where the chosen floor is
//
// There is no button direction to take a heading from, so it has to come
// from the floor's position relative to the car. Test 6 covers up; this
// covers down.
void test_heads_down_when_the_chosen_floor_is_below() {
    Elevator elevator(10, 1);
    elevator.request({6, Direction::UP});
    for (int i = 0; i < 6; ++i) {
        elevator.step();
    }
    CHECK_EQ("parked at 6", elevator.current_floor, 6);

    elevator.select_floor(2);
    for (int i = 0; i < 6; ++i) {
        elevator.step();
    }
    CHECK_EQ("went down to 2 and stayed", elevator.current_floor, 2);
    CHECK_FALSE("2 goes dark once served", elevator.is_selected(2));
}

// 9. A floor the building does not have is ignored, not stored
void test_ignores_a_floor_the_building_does_not_have() {
    Elevator elevator(10, 1);
    elevator.select_floor(11);
    elevator.select_floor(0);
    CHECK_FALSE("11 is not a floor", elevator.is_selected(11));
    CHECK_FALSE("0 is not a floor", elevator.is_selected(0));

    for (int i = 0; i < 3; ++i) {
        elevator.step();
    }
    CHECK_EQ("nowhere to go", elevator.current_floor, 1);
}

// 10. Floors the registers cannot hold are brought into line, not indexed
void test_keeps_the_building_above_ground() {
    Elevator elevator(5, -2);
    CHECK_EQ("basement clamped to 0", elevator.current_floor, 0);

    elevator.request({-1, Direction::UP});
    elevator.step();
    CHECK_EQ("a call below 0 goes nowhere", elevator.current_floor, 0);
}

// 11. A top below the bottom becomes a one-floor building rather than a crash
void test_survives_a_top_below_the_bottom() {
    Elevator elevator(-5, 0);
    elevator.step();
    CHECK_EQ("one floor, nothing to do", elevator.current_floor, 0);
}

void run_elevator_tests() {
    test_initial_state();
    test_moves_one_floor_per_step();
    test_stops_at_destination();
    test_serves_calls_in_travel_order();
    test_reverses_when_nothing_is_ahead();
    test_takes_a_passenger_to_the_floor_they_chose();
    test_lets_a_passenger_off_whichever_way_it_is_heading();
    test_heads_down_when_the_chosen_floor_is_below();
    test_ignores_a_floor_the_building_does_not_have();
    test_keeps_the_building_above_ground();
    test_survives_a_top_below_the_bottom();
}
