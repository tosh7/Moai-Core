// The 2D physics core the emoji pit runs on.
//
// Not built yet: add this file and tests/test_world.cpp to test.sh, and
// uncomment run_world_tests() in tests/main.cpp, once step() can satisfy them.

#include "world.h"

World::World(float w, float h) {
    width = w;
    height = h;
    gravity = {0, 0};
}