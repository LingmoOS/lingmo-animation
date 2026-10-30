# lingmo-animation

Animation primitives for the Lingmo desktop, shared by its shell, windows and
applications.

The library is deliberately small and standalone:

- **no toolkit, no window, no clock.** A value is stepped with `advance(dtMs)`
  by whoever owns the frames (a `ui::Window`, a compositor repaint, a test),
  so the same curve runs everywhere and the timing is exact and testable.
- **no dependencies.** C++20 standard library only, header only.
- **built from what the desktop already needed.** The dock's magnification
  smoothing, its launch and attention hops, the launcher's opening curve and
  the notification motion each carry their own copy of this math today; this
  library is where they will share it.

## What is inside

| Header | Primitive |
| --- | --- |
| `lingmo/animation/curve.h` | `Curve` easing functions (linear, quad, cubic, quart, quint, sine, back), custom curve functions, `lerp` |
| `lingmo/animation/tween.h` | `Tween`: a value travelling to a target over a duration, with continuous retargeting |
| `lingmo/animation/smoothing.h` | `Smoothing`: exponential approach to a target that can move every frame |
| `lingmo/animation/spring.h` | `Spring`: a damped spring with velocity-preserving retargeting and a stable integrator |
| `lingmo/animation/hop.h` | `Hop`: a few decreasing hops for dock launch and attention bounces |
| `lingmo/animation/timeline.h` | `Timeline`: phases in sequence, or beside each other |
| `lingmo/animation/stepper.h` | `Stepper`: monotonic nanosecond timestamps to a clamped frame delta |
| `lingmo/animation/animation.h` | Everything above |

All primitives implement `Advanceable`:

```cpp
class Advanceable {
public:
    virtual ~Advanceable() = default;
    virtual bool advance(double dtMs) = 0;  // true when it changed
    virtual bool running() const = 0;
};
```

`onFinished` is called once, when `advance()` reaches the end.

## Example

```cpp
#include <lingmo/animation/animation.h>

using namespace Lingmo::animation;

Tween opacity(0.0, 160.0, Curve::OutCubic);
opacity.setTarget(1.0);

// From a frame callback; the driver owns the clock.
double dtMs = stepper.tick(frameTimeNs);
if (opacity.advance(dtMs)) {
    view->setOpacity(opacity.value());
    // Ask for another frame while it runs.
    if (opacity.running())
        window->scheduleFrame();
}
```

Driver integration is intentionally trivial: keep a list of `Advanceable*`,
step them all on each frame, and repaint when one changes.

## Building and testing

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The tests are plain C++ (no test framework) and run on any machine: the
primitives are pure arithmetic.

## Using it

```cmake
find_package(LingmoAnimation REQUIRED)
target_link_libraries(my-target PRIVATE Lingmo::Animation)
```

or with pkg-config:

```sh
pkg-config --cflags lingmo-animation
```

## Status and next steps

This repository starts as the standalone home of the primitives, so the
existing components can adopt it one at a time without a mixed change:

1. add a `ui::MotionDriver` (`FrameClient`) in `lingmo-sdk` and move
   `ui::Animation` onto `Tween`;
2. move the dock's magnification, bounce and presentation math in
   `lingmo-compositor` onto `Smoothing`, `Hop` and `Tween`;
3. replace the duplicated easing helpers in the compositor's scene, session,
   window manager, launcher and notification code with `evaluate()`.

## License

BSD-3-Clause. See `LICENSE`.
