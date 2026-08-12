#include "check.h"

void run_elevator_tests();

// The physics suite in test_world.cpp is written but not yet wired up: World
// has no implementation to link against. Add source/world.cpp and
// tests/test_world.cpp back to test.sh, then uncomment these two lines.
// void run_world_tests();

int main() {
    run_elevator_tests();
    // run_world_tests();

    std::cout << checks - failures << "/" << checks << " checks passed"
              << std::endl;
    return failures == 0 ? 0 : 1;
}
