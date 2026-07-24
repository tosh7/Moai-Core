#include "elevator.h"
#include "directions.h"
#include <vector>

Elevator::Elevator(int max, int min) {
    max_floor = max;
    min_floor = min;
    current_floor = 1;
};

void Elevator::request(const Request& call) {
    if(call.floor > max_floor || call.floor < min_floor) {
        return;
    }

    for(const Request& r : requests) {
        if(r == call) return;
    }

    requests.push_back(call);
};

void Elevator::step() {
    if(requests.empty()) {
        m_direction = std::nullopt;
        return;
    }

    // Serve requests in the order they arrived: the head is the current target.
    const int target = requests.front().floor;

    if(current_floor < target) {
        m_direction = Direction::UP;
        current_floor++;
    } else if(current_floor > target) {
        m_direction = Direction::DOWN;
        current_floor--;
    }

    if(current_floor == target) {
        requests.erase(requests.begin());
    }
};

