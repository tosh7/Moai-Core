#include "elevator.h"
#include "directions.h"
#include <vector>

Elevator::Elevator(int max, int min) {
    max_floor = max;
    min_floor = min;
    current_floor = 1;
};

void Elevator::request(int floor, Direction direction) {
    if(floor > max_floor || floor < min_floor) { 
        return;
    }

    for(const Request& r : requested_floors) {
        if(r == Request{floor, direction}) return; 
    }

    requested_floors.push_back({floor, direction});
};

void Elevator::step() {
};

