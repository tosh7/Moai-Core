#include "water_game.h"

#include <algorithm>
#include <cmath>

// Rings sink slowly through water: gravity pulls, drag holds them back, and
// they settle at kSink / kDrag, a leisurely 100 a second.
constexpr float kSink = 1000;
constexpr float kDrag = 10;

// Thin enough to pass through any ring's hole.
constexpr float kPegThickness = 6;

// A jet at full strength, and how fast a burst dies away: a press is a
// short push, gone within about half a second.
constexpr float kJetSpeed = 1200;
constexpr float kJetReach = 500;
constexpr float kJetWidth = 60;
constexpr float kJetFade = 4;

// A ring on a peg is seen edge on, so each one stacks this much higher than
// the last; it slides down to its place at this speed.
constexpr float kRingStack = 6;
constexpr float kSlide = 300;

WaterGame::WaterGame(float width, float height): world(width, height) {
    world.set_gravity({0, -kSink});
    world.set_drag(kDrag);
}

int WaterGame::add_ring(Vec2 position, float radius, float hole) {
    int body = world.add_body(position, radius);
    rings.push_back({body, radius, hole, -1, 0});
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

Vec2 WaterGame::ring_position(int ring) const {
    return world.position_of(rings[ring].body);
}

int WaterGame::ring_on_peg(int ring) const {
    return rings[ring].peg;
}

int WaterGame::rings_on(int peg) const {
    return pegs[peg].count;
}

float WaterGame::jet_strength(int jet) const {
    return world.flow_strength(jets[jet].flow);
}

void WaterGame::pump(int jet) {
    world.set_flow(jets[jet].flow, 1);
}

void WaterGame::set_down(Vec2 direction) {
    // Only the direction counts; how hard the rings sink is the game's.
    // A device lying flat reports no direction at all, and is ignored.
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length == 0) {
        return;
    }
    world.set_gravity({direction.x / length * kSink, direction.y / length * kSink});
}

void WaterGame::step(float dt) {
    // A press is a burst, not a valve: every jet dies away on its own.
    for (const Jet& jet : jets) {
        float strength = world.flow_strength(jet.flow);
        strength *= std::max(0.0f, 1 - kJetFade * dt);
        world.set_flow(jet.flow, strength < 0.01f ? 0 : strength);
    }

    world.step(dt);

    // A peg is a thin obstacle, so World has already stopped a ring that
    // came down on its tip and left it sitting there. The ring is caught if
    // the tip is inside its hole: sitting on the tip, and near enough the
    // middle that the peg passes through rather than catching the rim.
    for (Ring& ring : rings) {
        if (ring.peg >= 0) {
            continue;
        }
        Vec2 at = world.position_of(ring.body);
        for (int index = 0; index < static_cast<int>(pegs.size()); index++) {
            Peg& peg = pegs[index];
            float off = std::abs(at.x - peg.tip.x);
            float above = at.y - peg.tip.y;
            if (off + kPegThickness / 2 >= ring.hole) {
                continue;
            }
            if (above < 0 || above > ring.radius + 2) {
                continue;
            }
            ring.peg = index;
            ring.rest = peg.tip.y - peg.length + kRingStack * (peg.count + 0.5f);
            peg.count++;
            world.hold_body(ring.body);
            world.move_body(ring.body, {peg.tip.x, at.y});
            break;
        }
    }

    // A caught ring slides down the peg to rest on the one below it.
    for (const Ring& ring : rings) {
        if (ring.peg < 0) {
            continue;
        }
        Vec2 at = world.position_of(ring.body);
        if (at.y <= ring.rest) {
            continue;
        }
        float y = std::max(ring.rest, at.y - kSlide * dt);
        world.move_body(ring.body, {pegs[ring.peg].tip.x, y});
    }
}
