// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <cmath>
#include <functional>

#include "lingmo/animation/advanceable.h"
#include "lingmo/animation/curve.h"

namespace Lingmo::animation {

// A few hops of decreasing height, the shape of the dock's launch and
// attention bounces: lift = amplitude * |sin(pi * t / period)| * e^(-decay * t).
// It is an explicit shape rather than a spring because the dock wants exactly
// the same motion every time an application starts.
class Hop : public Advanceable
{
public:
    struct Parameters {
        // The peak of the first hop, as a fraction of the size given to start().
        double amplitude = 0.18;
        // Time from the ground to the peak and back.
        double periodMs = 300.0;
        int hops = 3;
        // Larger settles faster; 1.1 over the whole run.
        double decay = 1.1;
    };

    Hop() = default;
    explicit Hop(Parameters parameters) : m_parameters(parameters) {}

    void setParameters(Parameters parameters) { m_parameters = parameters; }
    const Parameters &parameters() const { return m_parameters; }

    // Starts a hop for a `size` long icon.
    void start(double size) { startWithAmplitude(size * m_parameters.amplitude); }

    // Starts a hop whose first peak is `amplitude` pixels.
    void startWithAmplitude(double amplitude)
    {
        m_amplitude = std::max(0.0, amplitude);
        m_elapsed = 0.0;
        m_running = m_amplitude > 0.0 && m_parameters.hops > 0 && m_parameters.periodMs > 0.0;
    }

    void stop()
    {
        m_running = false;
        m_elapsed = durationMs();
    }

    double durationMs() const
    {
        return std::max(0.0, m_parameters.periodMs) * std::max(0, m_parameters.hops);
    }

    // How far above the rest position the value is now, 0 at the ground.
    double lift() const
    {
        if (!m_running)
            return 0.0;
        const double duration = durationMs();
        if (m_elapsed <= 0.0 || m_elapsed >= duration)
            return 0.0;
        return m_amplitude * std::abs(std::sin(kPi * m_elapsed / m_parameters.periodMs))
                * std::exp(-m_parameters.decay * m_elapsed / duration);
    }

    bool running() const override { return m_running; }

    bool advance(double dtMs) override
    {
        if (!m_running)
            return false;
        const double dt = std::max(0.0, dtMs);
        m_elapsed += dt;
        if (m_elapsed >= durationMs()) {
            m_elapsed = durationMs();
            m_running = false;
            if (onFinished)
                onFinished();
            return true;
        }
        return dt > 0.0;
    }

    // Called once when the last hop lands.
    std::function<void()> onFinished;

private:
    Parameters m_parameters;
    double m_amplitude = 0.0;
    double m_elapsed = 0.0;
    bool m_running = false;
};

} // namespace Lingmo::animation
