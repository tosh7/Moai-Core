#include "check.h"

void run_blower_tests();
void run_elevator_tests();
void run_fft_tests();
void run_voice_changer_tests();
void run_world_tests();

int main() {
    run_blower_tests();
    run_elevator_tests();
    run_fft_tests();
    run_voice_changer_tests();
    run_world_tests();

    std::cout << checks - failures << "/" << checks << " checks passed"
              << std::endl;
    return failures == 0 ? 0 : 1;
}
