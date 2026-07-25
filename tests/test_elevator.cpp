#include "../source/elevator.h"
#include <iostream>

static int checks = 0;
static int failures = 0;

#define CHECK_EQ(label, actual, expected)                                     \
    do {                                                                      \
        ++checks;                                                             \
        auto a = (actual);                                                    \
        auto e = (expected);                                                  \
        if (a != e) {                                                         \
            ++failures;                                                       \
            std::cout << "FAIL: " << label << " - expected " << e             \
                      << ", got " << a << std::endl;                          \
        }                                                                     \
    } while (0)

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

int main() {
    test_initial_state();
    test_moves_one_floor_per_step();
    test_stops_at_destination();
    test_serves_calls_in_travel_order();
    test_reverses_when_nothing_is_ahead();

    std::cout << checks - failures << "/" << checks << " checks passed"
              << std::endl;
    return failures == 0 ? 0 : 1;
}
