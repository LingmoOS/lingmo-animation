// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <cstdint>

namespace Lingmo::animation {

// Turns a monotonic timestamp in nanoseconds into a step in milliseconds for
// a driver. The first tick after reset() reports no step: there is no interval
// before it to measure. A long stall (a suspended machine, a debugger) is
// clamped so animations resume instead of teleporting.
class Stepper
{
public:
    explicit Stepper(double maxStepMs = 100.0)
        : m_maxStepMs(std::max(0.0, maxStepMs))
    {
    }

    void reset()
    {
        m_started = false;
        m_lastNs = 0;
    }

    void setMaxStep(double maxStepMs) { m_maxStepMs = std::max(0.0, maxStepMs); }

    double tick(uint64_t timeNs)
    {
        if (!m_started) {
            m_started = true;
            m_lastNs = timeNs;
            return 0.0;
        }
        if (timeNs <= m_lastNs)
            return 0.0;
        const double dtMs = double(timeNs - m_lastNs) / 1e6;
        m_lastNs = timeNs;
        return m_maxStepMs > 0.0 ? std::min(dtMs, m_maxStepMs) : dtMs;
    }

private:
    uint64_t m_lastNs = 0;
    bool m_started = false;
    double m_maxStepMs = 100.0;
};

} // namespace Lingmo::animation
