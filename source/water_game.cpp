#include "water_game.h"

// Rings sink slowly through water: gravity pulls, drag holds them back, and
// they settle at kSink / kDrag, a leisurely 200 a second.
constexpr float kSink = 1000;
constexpr float kDrag = 5;

WaterGame::WaterGame(float width, float height): world(width, height) {
    world.set_gravity({0, -kSink});
    world.set_drag(kDrag);
}