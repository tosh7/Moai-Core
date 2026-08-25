#include "elevator.h"
#include "directions.h"
#include <vector>

Elevator::Elevator(int max, int min) {
    max_floor = max;
    min_floor = min;
    up_calls.assign(max+1, false);
    down_calls.assign(max+1, false);
    car_calls.assign(max+1, false);
    current_floor = 1;
};

void Elevator::request(const Request& call) {
    if(call.floor > max_floor || call.floor < min_floor) {
        return;
    }

    switch (call.direction) {
        using enum Direction;
        case UP:
            up_calls[call.floor] = true;
            break;
        case DOWN:
            down_calls[call.floor] = true;
            break;
    }
};

void Elevator::step() {
    // Look for work on either side. Whether a call is up or down does not
    // matter here: an opposite-direction call still has to be travelled to.
    bool has_above = false;
    for (int i = current_floor + 1; i <= max_floor; i++) {
        if (up_calls[i] || down_calls[i]) {
            has_above = true;
            break;
        }
    }

    bool has_below = false;
    for (int i = current_floor - 1; i >= min_floor; i--) {
        if (up_calls[i] || down_calls[i]) {
            has_below = true;
            break;
        }
    }

    // Serve the calls this floor can honour. While travelling, only the call
    // matching our heading boards; the opposite one boards too when nothing
    // lies ahead, because this floor is where the run turns around.
    if (!m_direction.has_value()) {
        if (up_calls[current_floor]) {
            m_direction = Direction::UP;
            up_calls[current_floor] = false;
        } else if (down_calls[current_floor]) {
            m_direction = Direction::DOWN;
            down_calls[current_floor] = false;
        }
    } else if (m_direction == Direction::UP) {
        up_calls[current_floor] = false;
        if (!has_above) {
            down_calls[current_floor] = false;
            m_direction = has_below ? std::optional(Direction::DOWN) : std::nullopt;
        }
    } else if (m_direction == Direction::DOWN) {
        down_calls[current_floor] = false;
        if (!has_below) {
            up_calls[current_floor] = false;
            m_direction = has_above ? std::optional(Direction::UP) : std::nullopt;
        }
    }

    // Start a parked car moving when the only calls are on other floors.
    if (!m_direction.has_value()) {
        for (int i = min_floor; i <= max_floor; i++) {
            if (up_calls[i] || down_calls[i]) {
                m_direction = i > current_floor ? Direction::UP : Direction::DOWN;
                break;
            }
        }
    }

    if (!m_direction.has_value()) {
        return;
    } else switch (*m_direction) {
        using enum Direction;
        case UP:
            current_floor++;
            break;
        case DOWN:
            current_floor--;
            break;
    }
};

