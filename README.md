# Moai-Core
Core algorithms for Moai framework.

[![Test](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml/badge.svg)](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml)

[日本語](README.ja.md)

## Environments
Version: C++23

Building needs CMake 3.20 or newer. Ninja is used when it is installed and
make is used when it is not, so neither has to be set up first.

## Elevator

A single-car elevator that decides where to go from the calls waiting for it.

The core holds no timer and starts no threads. Nothing moves until the host
calls `step()`, so the caller owns the clock: drive it from a `Timer`, from
`CADisplayLink`, or as fast as a test wants to run.

```cpp
#include "elevator.h"

Elevator elevator(10, 1);                 // floors 1 through 10, parked on 1

elevator.request({5, Direction::UP});     // up button pressed on floor 5

elevator.step();                          // one call, one floor travelled
elevator.current_floor;                   // 2
```

`request` records a hall call — the floor a button was pressed on, and the
direction that caller wants to travel. Pressing up and down on the same floor
registers two separate calls. Calls outside the building are ignored, and
pressing the same button twice changes nothing.

`step` advances the car by one floor. It keeps its heading while any call
remains ahead of it and reverses once none does, so a car running up from 1
to 5 stops at 3 on the way rather than doubling back for it. Only the call
matching the car's heading boards, except on the floor where the run turns
around: with nothing further ahead, the opposite call boards there too.

Not yet supported: car calls (buttons inside the car), door state, basement
floors, and dispatching more than one car.

## World

A pit of circular bodies that fall, collide and pile up.

Like the elevator, it has no clock. `step(dt)` advances the simulation by one
fixed slice, and the host decides how often that happens, so the same run can
be watched at a comfortable pace on screen or replayed in an instant in a test.
A fixed slice also makes the result repeatable: the same bodies dropped from
the same places land the same way every time.

```cpp
#include "world.h"

World world(400, 800);                       // a pit 400 by 800, floor at y = 0
world.set_gravity({0, -1000});               // y points up, so down is negative

int body = world.add_body({200, 700}, 20);   // a circle of radius 20

world.step(1.0f / 60);                       // one slice of simulated time
world.position_of(body);                     // where to draw it now
```

`add_body` returns an index rather than a pointer, so a handle stays valid as
the pit fills up. Bodies are circles: there is no other shape, and no rotation.

`step` integrates gravity, pushes overlapping bodies apart, reflects the speed
at which they met, and keeps everything inside the walls. A pair that has
merely sagged together under gravity does not bounce; only a real impact does.

`apply_radial_impulse` shoves everything within reach of a point directly away
from it, harder the closer it is — a tap that scatters a pile.

Not yet supported: shapes other than circles, rotation, per-body mass or
material, and joints.

## How to build
To make .a file, do below.
1. Clone this repository
2. run `sh build.sh`

You will get `build/ios/libmoai.a`.

The repository is also a Swift package, so it can be consumed directly by
Xcode or by another `Package.swift`.

## How to test
Run `sh test.sh`. It builds the sources together with `tests/` and runs them,
printing how many checks passed and exiting non-zero on failure.

## License
[MIT License](https://github.com/tosh7/Moai-Core/blob/main/LICENSE)

## Contact me
- e-mail: zlia.6.lj.425@gmail.com
- Twitter: [tosh_3](https://x.com/tosh_3)
- Linked In: [Satoshi Komatsu](https://www.linkedin.com/in/satoshi-komatsu-5a8a4a220/)
