#include "directions.h"

#ifndef Elevator_H
#define Elevator_H

class Elevator {
    public:
        Elevator(int max, int min);
        void request(int floor, Direction direction);
    private:
        int max_floor;
        int min_floor;
};

#endif