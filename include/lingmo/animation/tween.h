// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <functional>

#include "lingmo/animation/advanceable.h"
#include "lingmo/animation/curve.h"

namespace Lingmo::animation {

// A value transitioning to a target over a duration. Setting a new target
// while it runs keeps the value where it is and travels from there, so an
// interrupted animation never jumps: the dock's glass and the launcher's
// panels both rely on that.
class Tween : public Advanceable
{
public:
    Tween() = default;
    explicit Tween(double value, double durationMs = 250.0,
                   Curve curve = Curve::InOutCubic)
        : m_value(value)
        , m_from(value)
        , m_target(value)
        , m_duration(std::max(0.0, durationMs))
        , m_curve(curve)
    {
    }

    void setDuration(double durationMs)
    {
        m_duration = std::max(0.0, durationMs);
        if (m_running && m_duration <= 0.0)
            finish();
    }
    double duration() const { return m_duration; }

    void setCurve(Curve curve) { m_curve = curve; }
    Curve curve() const { return m_curve; }

    // Jumps to a value; there is nothing to animate and nothing to finish.
    void setValue(double value)
    {
        m_value = value;
        m_from = value;
        m_target = value;
        m_elapsed = 0;
        m_running = false;
    }
    double value() const { return m_value; }

    // Travels from the current value to `target`, over the duration.
    void setTarget(double target)
    {
        m_from = m_value;
        m_target = target;
        m_elapsed = 0;
        m_running = m_duration > 0.0 && m_from != target;
        if (!m_running)
            m_value = target;
    }
    double target() const { return m_target; }

    bool running() const override { return m_running; }

    // How far the current travel has gone, 0..1.
    double progress() const
    {
        return m_duration > 0.0 ? clamp01(m_elapsed / m_duration) : (m_running ? 0.0 : 1.0);
    }

    // Freezes where it is.
    void stop()
    {
        m_target = m_value;
        m_from = m_value;
        m_elapsed = 0;
        m_running = false;
    }

    // Jumps to the target without calling onFinished.
    void finish()
    {
        m_value = m_target;
        m_running = false;
    }

    bool advance(double dtMs) override
    {
        if (!m_running)
            return false;
        m_elapsed += std::max(0.0, dtMs);
        const double progress = m_duration > 0.0 ? clamp01(m_elapsed / m_duration) : 1.0;
        const double previous = m_value;
        m_value = m_from + (m_target - m_from) * evaluate(m_curve, progress);
        const bool changed = m_value != previous;
        if (progress >= 1.0) {
            m_value = m_target;
            m_running = false;
            if (onFinished)
                onFinished();
        }
        return changed || !m_running;
    }

    // Called once when advance() reaches the target.
    std::function<void()> onFinished;

private:
    double m_value = 0.0;
    double m_from = 0.0;
    double m_target = 0.0;
    double m_duration = 250.0;
    double m_elapsed = 0.0;
    Curve m_curve = Curve::InOutCubic;
    bool m_running = false;
};

} // namespace Lingmo::animation
