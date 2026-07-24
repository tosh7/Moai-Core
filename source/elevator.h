#ifndef Elevator_H
#define Elevator_H

#include "directions.h"
#include <vector>
#include <optional>

struct Request {
    int floor;
    Direction direction;

    bool operator==(const Request&) const = default;
};

class Elevator {
    public:
        int current_floor;
        Elevator(int max, int min);
        void request(int floor, Direction direction);
        void step();
    private:
        int max_floor;
        int min_floor;
        std::vector<Request> requested_floors;
        std::optional<Direction> m_direction;
};                  

#endif