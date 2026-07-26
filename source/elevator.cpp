#include "elevator.h"
#include "directions.h"
#include <vector>

Elevator::Elevator(int max, int min) {
    max_floor = max;
    min_floor = min;
    up_calls.assign(max+1, false);
    down_calls.assign(max+1, false);
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
};
