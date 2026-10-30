// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <cmath>
#include <functional>

#include "lingmo/animation/advanceable.h"

namespace Lingmo::animation {

// A value chasing a target with exponential approach. Unlike Tween it has no
// duration: it is what a pointer-driven animation wants (dock magnification,
// hover glows), because a target that moves every frame is followed smoothly
// instead of restarting a transition each time.
class Smoothing : public Advanceable
{
public:
    explicit Smoothing(double value = 0.0, double timeConstantMs = 90.0,
                       double epsilon = 5e-4)
        : m_value(value)
        , m_target(value)
        , m_timeConstant(std::max(0.0, timeConstantMs))
        , m_epsilon(std::max(0.0, epsilon))
    {
    }

    // The time to cover ~63% of the remaining distance.
    void setTimeConstant(double timeConstantMs)
    {
        m_timeConstant = std::max(0.0, timeConstantMs);
    }
    double timeConstant() const { return m_timeConstant; }

    // Distance below which the value snaps to its target and stops.
    void setEpsilon(double epsilon) { m_epsilon = std::max(0.0, epsilon); }
    double epsilon() const { return m_epsilon; }

    void setValue(double value)
    {
        m_value = value;
        m_target = value;
        m_running = false;
    }
    double value() const { return m_value; }

    void setTarget(double target)
    {
        m_target = target;
        if (m_timeConstant <= 0.0) {
            m_value = target;
            m_running = false;
        } else {
            m_running = m_value != target;
        }
    }
    double target() const { return m_target; }

    bool running() const override { return m_running; }

    bool advance(double dtMs) override
    {
        if (!m_running)
            return false;
        const double previous = m_value;
        const double dt = std::max(0.0, dtMs);
        if (m_timeConstant <= 0.0) {
            snap();
            return true;
        }
        m_value += (m_target - m_value) * (1.0 - std::exp(-dt / m_timeConstant));
        if (std::abs(m_target - m_value) <= m_epsilon) {
            m_value = m_target;
            m_running = false;
            if (onFinished)
                onFinished();
        }
        return m_value != previous || !m_running;
    }

    // Jumps to the target; no onFinished.
    void snap()
    {
        m_value = m_target;
        m_running = false;
    }

    // Called once when advance() snaps to the target.
    std::function<void()> onFinished;

private:
    double m_value = 0.0;
    double m_target = 0.0;
    double m_timeConstant = 90.0;
    double m_epsilon = 5e-4;
    bool m_running = false;
};

} // namespace Lingmo::animation
