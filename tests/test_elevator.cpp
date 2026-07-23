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
    elevator.request(5, Direction::UP);
    for (int expected = 2; expected <= 5; ++expected) {
        elevator.step();
        CHECK_EQ("moving up to floor " + std::to_string(expected), elevator.current_floor, expected);
    }
}

// 3. Once arrived, the request clears and it moves no further
void test_stops_at_destination() {
    Elevator elevator(10, 1);
    elevator.request(3, Direction::UP);
    for (int i = 0; i < 5; ++i) {
        elevator.step();
    }
    CHECK_EQ("stays at destination", elevator.current_floor, 3);
}

int main() {
    test_initial_state();
    test_moves_one_floor_per_step();
    test_stops_at_destination();

    std::cout << checks - failures << "/" << checks << " checks passed"
              << std::endl;
    return failures == 0 ? 0 : 1;
}
