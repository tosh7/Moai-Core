# Moai-Core
Core algorithms for Moai framework.

[日本語](README.ja.md)

## Environments
Version: C++23

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

## How to build
To make .a file, do below.
1. Clone this repository
2. run `sh build.sh`

You will get .a file output under build directory.

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
