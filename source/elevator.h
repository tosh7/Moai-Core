#include "directions.h"

#ifndef Elevator_H
#define Elevator_H

class Elevator {
    public:
        Elevator(max: int, minFloor: int);
        void request(floor: int, direction: Direction);
    private:
        int max_floor;
        int min_floor;
};

#endif