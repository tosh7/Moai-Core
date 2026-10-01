#include "water_game.h"

// Rings sink slowly through water: gravity pulls, drag holds them back, and
// they settle at kSink / kDrag, a leisurely 200 a second.
constexpr float kSink = 1000;
constexpr float kDrag = 5;

// Thin enough to pass through any ring's hole.
constexpr float kPegThickness = 6;
constexpr float kJetSpeed = 1200;
constexpr float kJetReach = 500;
constexpr float kJetWidth = 60;

WaterGame::WaterGame(float width, float height): world(width, height) {
    world.set_gravity({0, -kSink});
    world.set_drag(kDrag);
}

int WaterGame::add_ring(Vec2 position, float radius, float hole) {
    int body = world.add_body(position, radius);
    rings.push_back({body, hole, -1});
    return static_cast<int>(rings.size()) - 1;
}

int WaterGame::add_peg(Vec2 tip, float length) {
    int obstacle = world.add_obstacle(
        {tip.x, tip.y - length / 2},
        {kPegThickness / 2, length / 2},
        0);
    pegs.push_back({obstacle, tip, length, 0});
    return static_cast<int>(pegs.size()) - 1;
}

int WaterGame::add_jet(Vec2 nozzle) {
    int flow = world.add_flow(nozzle, {0, kJetSpeed}, kJetReach, kJetWidth);
    jets.push_back({flow});
    return static_cast<int>(jets.size() - 1);
}
