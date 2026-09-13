#include "blower.h"

#include <algorithm>

// A spring: the curl pulls back, breath pushes out. One stiffness for both,
// so a full breath holds it fully unrolled.
constexpr float stiffness = 100.0f;

// Under 2 * sqrt(stiffness * mass) it bounces on the way back, as the toy
// does. This is half that.
constexpr float friction = 10.0f;

constexpr float mass = 1.0f;

Blower::Blower(float length) {
    this->length = std::max(length, 1.0f);
    breath = 0;
    m_extension = 0;
    m_velocity = 0;
}

void Blower::set_breath(float strength){
    breath = std::clamp(strength, 0.0f, 1.0f);
}

void Blower::step(float dt) {
    // Three forces, then the same semi-implicit Euler as World.
    float push = breath * stiffness * length;
    float pull = -m_extension * stiffness;
    float drag = -m_velocity * friction;

    float acceleration = (push + pull + drag) / mass;
    m_velocity += acceleration * dt;
    m_extension += m_velocity * dt;

    // Stop dead at either end rather than push against it.
    float clamped = std::clamp(m_extension, 0.0f, length);
    if (clamped != m_extension) {
        m_extension = clamped;
        m_velocity = 0;
    }
}

float Blower::extension() const {
    return m_extension;
}