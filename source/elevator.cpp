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
};

