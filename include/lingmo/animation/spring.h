// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <cmath>
#include <functional>

#include "lingmo/animation/advanceable.h"

namespace Lingmo::animation {

// A damped spring. Retargeting keeps the velocity, so a moving value bends
// towards its new target instead of jumping, which is what icons landing in a
// row and popups pushed aside want.
class Spring : public Advanceable
{
public:
    struct Parameters {
        // acceleration = (-stiffness * distance - damping * velocity) / mass
        double stiffness = 170.0;
        double damping = 26.0;
        double mass = 1.0;
        // Below both thresholds the value is at the target: the spring stops.
        double restDistance = 0.01;
        double restVelocity = 0.01;
        // The integrator substeps to at most this, so a long frame cannot
        // throw the spring off.
        double maxStepMs = 1000.0 / 240.0;
    };

    Spring() = default;
    explicit Spring(double value)
        : Spring(value, Parameters {})
    {
    }
    explicit Spring(double value, Parameters parameters)
        : m_value(value)
        , m_target(value)
        , m_parameters(parameters)
    {
    }

    void setParameters(Parameters parameters)
    {
        m_parameters = parameters;
        if (!(m_parameters.stiffness > 0.0) || !(m_parameters.mass > 0.0))
            snap();
    }
    const Parameters &parameters() const { return m_parameters; }

    void setValue(double value)
    {
        m_value = value;
        m_target = value;
        m_velocity = 0.0;
        m_running = false;
    }
    double value() const { return m_value; }

    void setTarget(double target)
    {
        m_target = target;
        if (!(m_parameters.stiffness > 0.0) || !(m_parameters.mass > 0.0)) {
            snap();
            return;
        }
        m_running = m_value != target || m_velocity != 0.0;
    }
    double target() const { return m_target; }
    double velocity() const { return m_velocity; }

    bool running() const override { return m_running; }

    bool advance(double dtMs) override
    {
        if (!m_running)
            return false;
        const double previous = m_value;
        double remaining = std::max(0.0, dtMs);
        const double maxStep = std::clamp(m_parameters.maxStepMs, 0.05, 100.0);
        while (remaining > 0.0) {
            const double step = std::min(remaining, maxStep) / 1000.0;
            const double acceleration =
                    (-m_parameters.stiffness * (m_value - m_target)
                     - m_parameters.damping * m_velocity)
                    / m_parameters.mass;
            m_velocity += acceleration * step;
            m_value += m_velocity * step;
            remaining -= std::min(remaining, maxStep);
        }
        if (std::abs(m_value - m_target) <= m_parameters.restDistance
            && std::abs(m_velocity) <= m_parameters.restVelocity) {
            m_value = m_target;
            m_velocity = 0.0;
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
        m_velocity = 0.0;
        m_running = false;
    }

    // Called once when advance() comes to rest.
    std::function<void()> onFinished;

private:
    double m_value = 0.0;
    double m_target = 0.0;
    double m_velocity = 0.0;
    Parameters m_parameters;
    bool m_running = false;
};

} // namespace Lingmo::animation
