#include "../source/include/blower.h"
#include "check.h"

namespace {

constexpr float kStep = 1.0f / 120.0f;
constexpr float kLength = 200.0f;

// Enough for the spring to settle whatever it was doing.
void run(Blower& blower, float seconds) {
    for (float t = 0; t < seconds; t += kStep) {
        blower.step(kStep);
    }
}

}  // namespace

// 1. Left alone, it stays rolled up
void test_stays_rolled_up_unblown() {
    Blower blower(kLength);
    run(blower, 1);
    CHECK_EQ("never moved", blower.extension(), 0.0f);
}

// 2. A steady breath settles it at a matching length and holds it there
void test_settles_in_proportion_to_the_breath() {
    Blower blower(kLength);
    blower.set_breath(0.5f);
    run(blower, 2);

    CHECK_NEAR("halfway out", blower.extension(), kLength / 2, 2);

    float before = blower.extension();
    run(blower, 0.5f);
    CHECK_NEAR("and holding", blower.extension(), before, 0.5f);
}

// 3. A full breath reaches the end and no further
void test_full_breath_reaches_full_length() {
    Blower blower(kLength);
    blower.set_breath(1);
    run(blower, 2);

    CHECK_NEAR("fully unrolled", blower.extension(), kLength, 1);
    CHECK_TRUE("not past the end", blower.extension() <= kLength);
}

// 4. When the breath stops it curls back and comes to rest
void test_curls_back_when_breath_stops() {
    Blower blower(kLength);
    blower.set_breath(1);
    run(blower, 2);

    blower.set_breath(0);
    run(blower, 2);
    CHECK_NEAR("rolled back up", blower.extension(), 0, 1);
}

// 5. Harder reaches further
void test_harder_breath_reaches_further() {
    Blower soft(kLength);
    Blower hard(kLength);
    soft.set_breath(0.3f);
    hard.set_breath(0.8f);
    run(soft, 2);
    run(hard, 2);

    CHECK_TRUE("hard went further", hard.extension() > soft.extension());
}

// 6. A shout is no more than a full breath, and sucking does nothing
//
// This checks the promise, not the clamp in set_breath: the end stop in step
// keeps it on its own, so the clamp is not visible from outside.
void test_clamps_the_breath() {
    Blower shouted(kLength);
    Blower full(kLength);
    shouted.set_breath(5);
    full.set_breath(1);
    run(shouted, 2);
    run(full, 2);

    CHECK_NEAR("a shout is a full breath", shouted.extension(), full.extension(), 0.01f);

    Blower sucked(kLength);
    sucked.set_breath(-1);
    run(sucked, 1);
    CHECK_EQ("sucking does nothing", sucked.extension(), 0.0f);
}

void run_blower_tests() {
    test_stays_rolled_up_unblown();
    test_settles_in_proportion_to_the_breath();
    test_full_breath_reaches_full_length();
    test_curls_back_when_breath_stops();
    test_harder_breath_reaches_further();
    test_clamps_the_breath();
}
