// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

namespace Lingmo::animation {

// A value that a driver moves a little every frame. The library has no clock
// of its own: a window, a compositor or a test hands it the time that passed,
// so the same primitive runs on the desktop's frame loops and in tests.
class Advanceable
{
public:
    virtual ~Advanceable() = default;

    // Steps by dtMs; true when the value changed (or the motion ended).
    virtual bool advance(double dtMs) = 0;

    // Whether more motion is left.
    virtual bool running() const = 0;
};

} // namespace Lingmo::animation
