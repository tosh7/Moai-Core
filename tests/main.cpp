#include "check.h"

void run_elevator_tests();
void run_world_tests();

int main() {
    run_elevator_tests();
    run_world_tests();

    std::cout << checks - failures << "/" << checks << " checks passed"
              << std::endl;
    return failures == 0 ? 0 : 1;
}
