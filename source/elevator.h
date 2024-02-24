#ifndef Elevator_H
#define Elevator_H

#include "directions.h"
#include <vector>

class Elevator {
    public:
        int current_floor;
        Elevator(int max, int min);
        void request(int floor, Direction direction);
    private:
        int max_floor;
        int min_floor;
        std::vector<int> requested_floors;
        Direction m_direction;
};                  

#endif