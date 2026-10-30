// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <cmath>

namespace Lingmo::animation {

inline constexpr double kPi = 3.14159265358979323846;

// The easing curves the desktop's animations share. Names follow the shape
// they draw over t in [0, 1]: Out starts fast and settles, InOut passes the
// middle at half the duration, In starts slow.
enum class Curve {
    Linear,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InQuart,
    OutQuart,
    InOutQuart,
    OutQuint,
    OutSine,
    InOutSine,
    OutBack,
};

// A hand-written curve, for one animation that needs its own shape.
using CurveFunction = double (*)(double);

inline constexpr double clamp01(double t)
{
    return std::clamp(t, 0.0, 1.0);
}

inline double evaluate(Curve curve, double t)
{
    t = clamp01(t);
    const double u = 1.0 - t;
    switch (curve) {
    case Curve::Linear:
        return t;
    case Curve::InQuad:
        return t * t;
    case Curve::OutQuad:
        return 1.0 - u * u;
    case Curve::InOutQuad:
        return t < 0.5 ? 2.0 * t * t : 1.0 - 2.0 * u * u;
    case Curve::InCubic:
        return t * t * t;
    case Curve::OutCubic:
        return 1.0 - u * u * u;
    case Curve::InOutCubic:
        return t < 0.5 ? 4.0 * t * t * t : 1.0 - 4.0 * u * u * u;
    case Curve::InQuart:
        return t * t * t * t;
    case Curve::OutQuart:
        return 1.0 - u * u * u * u;
    case Curve::InOutQuart:
        return t < 0.5 ? 8.0 * t * t * t * t : 1.0 - 8.0 * u * u * u * u;
    case Curve::OutQuint:
        return 1.0 - u * u * u * u * u;
    case Curve::OutSine:
        return std::sin(t * kPi / 2.0);
    case Curve::InOutSine:
        return 0.5 - 0.5 * std::cos(t * kPi);
    case Curve::OutBack: {
        // Overshoots a little past the target and comes back; the same shape
        // as the desktop's launcher opening.
        constexpr double c1 = 1.70158;
        constexpr double c3 = c1 + 1.0;
        const double s = t - 1.0;
        return 1.0 + c3 * s * s * s + c1 * s * s;
    }
    }
    return t;
}

inline double evaluate(CurveFunction function, double t)
{
    return function ? function(clamp01(t)) : clamp01(t);
}

// Linear interpolation, unclamped: a curve's output may legitimately pass the
// end for a moment, as OutBack does.
inline double lerp(double from, double to, double t)
{
    return from + (to - from) * t;
}

} // namespace Lingmo::animation
