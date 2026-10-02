# Moai-Core
Core algorithms for Moai framework.

[![Test](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml/badge.svg)](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml)

**Now on the App Store:** [The Moai](https://apps.apple.com/us/app/%E3%83%A2%E3%82%A2%E3%82%A4/id6804320896) — the elevator and the emoji pit, running on this core.

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
elevator.select_floor(8);                 // 8 pressed inside the car

elevator.step();                          // one call, one floor travelled
elevator.current_floor;                   // 2
elevator.is_selected(8);                  // true until the car gets there
```

`request` records a hall call — the floor a button was pressed on, and the
direction that caller wants to travel. Pressing up and down on the same floor
registers two separate calls. Calls outside the building are ignored, and
pressing the same button twice changes nothing.

`select_floor` records a car call — a floor chosen from inside. It has no
direction, because whoever pressed it is already aboard: they get off when the
car reaches that floor, whichever way it is heading. `is_selected` says whether
a floor is still lit, which is what a panel of buttons draws from.

`step` advances the car by one floor. It keeps its heading while any call —
hall or car — remains ahead of it and reverses once none does, so a car running up from 1
to 5 stops at 3 on the way rather than doubling back for it. Only the call
matching the car's heading boards, except on the floor where the run turns
around: with nothing further ahead, the opposite call boards there too.

Not yet supported: door state, basement floors, and dispatching more than one
car.

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
the pit fills up. Bodies are circles and do not rotate; for rectangles, see
obstacles below.

`step` integrates gravity, pushes overlapping bodies apart, reflects the speed
at which they met, and keeps everything inside the walls. A pair that has
merely sagged together under gravity does not bounce; only a real impact does.

`apply_radial_impulse` shoves everything within reach of a point directly away
from it, harder the closer it is — a tap that scatters a pile.

### Obstacles

Rectangles, at any angle, that bodies cannot enter.

```cpp
int shelf = world.add_obstacle({200, 300}, {100, 10}, 0);   // 200 wide, 20 tall, level
world.move_obstacle(shelf, {200, 320}, 0.1f);               // somewhere else, tilted

int mill = world.add_obstacle({200, 600}, {80, 8}, 0);
world.set_obstacle_spin(mill, 0);                           // free to turn
world.obstacle_angle(mill);                                 // where it has turned to
```

An obstacle is kept apart from the bodies: it never falls, never collides with
another obstacle, and never appears in `body_count`. A body lands on one and
rests there, and passes the ends of one as if it were not there. Only the part
of a body's motion heading into the obstacle is taken away; whatever it was
doing along the face, it keeps. An obstacle moved between steps carries along
what it touches.

An obstacle is held still until `set_obstacle_spin` is called on it, however
hard it is hit — which is what a shelf wants. Once free, whatever lands on it
sets it turning, harder the further from the middle it lands, and it throws
whatever it touches. It loses a little to friction as it goes, so a knock
spins it and then lets it settle.

### Drag and flows

What the bodies move through.

```cpp
world.set_drag(5);                                          // water; 0, the default, is air

int jet = world.add_flow({200, 0}, {0, 600}, 400, 40);      // from the floor, up at 600, 400 high, 40 wide
world.set_flow(jet, 1);                                     // 0 is off, 1 full; held until set again
```

Drag takes a share of a body's speed every second, so a body falling through
something thick stops gaining speed and sinks steadily at gravity over drag.

A flow is a column where the medium itself is moving, strongest at its origin
and fading to nothing at its reach and its edges. Drag pulls a body towards the
medium's velocity rather than towards rest, so still water slows a body and
moving water carries it. With no drag, a flow has nothing to carry with.

### Holding

```cpp
world.hold_body(body);                     // out of the physics, in the host's hands
world.move_body(body, finger);             // goes exactly here
world.release_body(body, {300, 0});        // back in, setting off at this velocity
```

A held body ignores gravity, drag and flows, and is not pushed out of obstacles
or walls: where the host put it is where it is. It is still there to the others,
which bump off it as off a wall. A finger dragging something, or a ring caught
on a peg.

Not yet supported: per-body mass or material, joints, and anything driving an
obstacle — a spin, once given, only runs down.

## Blower

A party blower: the paper toy that unrolls when blown into and curls back up
when let go.

```cpp
#include "blower.h"

Blower blower(200);                 // 200 long, fully unrolled

blower.set_breath(0.7f);            // from the mic, 0 through 1; held until set again
blower.step(1.0f / 120);            // one slice of time
blower.extension();                 // how far it is out, 0 through 200
```

It is a spring. Breath pushes it out, the curl of the paper pulls it back, and
friction settles it. A steady breath holds it at a matching length — half a
breath, halfway out; a full breath, fully unrolled — and letting go curls it
back with a small bounce, as the toy does. It stops dead at either end.

The core never hears any audio. The host reads the mic, turns loudness into a
breath strength, and hands that over, the way the pit hands over gravity from
the accelerometer.

Not yet supported: telling breath from speech, and a tip that bends.

## VoiceChanger

Shifts the pitch of a voice as it streams through, after the bow tie in
Detective Conan.

```cpp
#include "voice_changer.h"

VoiceChanger changer(48000, 1024);   // the host's sample rate; a 1024-sample window

changer.set_pitch(12);               // an octave up; -12 down; 0 leaves it alone
changer.process(in, out, count);     // with each buffer the host receives
```

Unlike the blower, the audio itself goes through the core: samples in,
samples out, in whatever buffer size the host's audio unit likes. The mic and
the speaker stay with the host. The output runs one window behind the input,
which at 1024 samples and 48 kHz is about 21 ms — the delay a host will hear.

Inside is a phase vocoder over a short-time Fourier transform. Each frame is
windowed, transformed, and every bin moved to the ratio of the shift; the
moved bin's phase is then run forward at the component's true rate, measured
from how its phase advanced since the last frame, so a tone between two bins
lands where it was sent rather than scattering either side of it.

`fft.h` holds the transform underneath, an in-place radix-2 Cooley–Tukey, as
two free functions. It is there for anything else that needs one.

Not yet supported: holding the formants still, so a shifted voice still sounds
like the same person rather than a smaller or larger one; and processing
without allocating, which an audio thread will want.

## WaterGame

The water ring toss: rings drifting in a sealed tank, two pumps in the floor,
and pegs to land them on. Built on World, which does the water.

```cpp
#include "water_game.h"

WaterGame game(400, 800);

int left = game.add_jet({100, 0});            // a pump in the floor
int peg  = game.add_peg({200, 300}, 200);     // tip at 300, standing 200 tall
int ring = game.add_ring({120, 700}, 20, 12); // outside 20, hole 12

game.pump(left);                              // one press: a burst that dies away
game.set_down({0.3f, -1});                    // tilting; only the direction is used
game.step(1.0f / 120);

game.ring_on_peg(ring);                       // the peg it is on, or -1
game.rings_on(peg);                           // the score
```

Rings sink slowly, as through water. A press sends a burst up from the pump
that lifts whatever is over it and dies away within half a second; holding the
button does no more than pressing it. A ring that comes down over a peg's tip,
with the tip inside its hole, is caught: it slides down the peg and rests on
whatever is already there. One that lands on its rim slips off.

The game decides how heavy the water is and how strong a pump is. The host
says only which way is down and when a button is pressed.

Not yet supported: rings coming back off a peg, pegs that are not upright, and
a caught ring that stops bumping into the others — on a peg it still collides
as a full circle.

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
