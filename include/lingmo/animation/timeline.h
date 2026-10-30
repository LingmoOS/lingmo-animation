// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "lingmo/animation/advanceable.h"
#include "lingmo/animation/curve.h"

namespace Lingmo::animation {

// Phases run one after another by default; together() puts a phase beside the
// one before it. Each phase gets its own curve and its progress 0..1, so a
// multi-part motion (the launcher's panel, then its list) is one object a
// driver can step.
class Timeline : public Advanceable
{
public:
    struct Phase {
        double durationMs = 0.0;
        Curve curve = Curve::Linear;
        std::function<void(double progress)> onUpdate;
        std::function<void()> onFinished;
    };

    // Appends a phase after everything added so far.
    Timeline &then(Phase phase)
    {
        Entry entry;
        entry.startMs = m_cursorMs;
        entry.phase = std::move(phase);
        m_anchorMs = entry.startMs;
        m_cursorMs = entry.startMs + std::max(0.0, entry.phase.durationMs);
        m_duration = std::max(m_duration, m_cursorMs);
        m_entries.push_back(std::move(entry));
        return *this;
    }

    // Starts a phase with the one before it (or the start of the timeline).
    Timeline &together(Phase phase)
    {
        Entry entry;
        entry.startMs = m_anchorMs;
        entry.phase = std::move(phase);
        m_cursorMs = std::max(m_cursorMs, entry.startMs + std::max(0.0, entry.phase.durationMs));
        m_duration = std::max(m_duration, m_cursorMs);
        m_entries.push_back(std::move(entry));
        return *this;
    }

    void clear()
    {
        m_entries.clear();
        m_duration = 0.0;
        m_elapsed = 0.0;
        m_cursorMs = 0.0;
        m_anchorMs = 0.0;
        m_running = false;
    }

    void start()
    {
        m_elapsed = 0.0;
        for (Entry &entry : m_entries)
            entry.finished = false;
        m_running = !m_entries.empty();
    }

    void stop() { m_running = false; }

    bool running() const override { return m_running; }
    double durationMs() const { return m_duration; }
    double elapsedMs() const { return m_elapsed; }

    bool advance(double dtMs) override
    {
        if (!m_running)
            return false;
        m_elapsed += std::max(0.0, dtMs);
        bool changed = dtMs > 0.0;
        for (Entry &entry : m_entries) {
            if (m_elapsed < entry.startMs)
                continue;
            const double duration = std::max(0.0, entry.phase.durationMs);
            const double progress = duration > 0.0
                    ? clamp01((m_elapsed - entry.startMs) / duration)
                    : 1.0;
            if (entry.phase.onUpdate)
                entry.phase.onUpdate(evaluate(entry.phase.curve, progress));
            changed = true;
            if (progress >= 1.0 && !entry.finished) {
                entry.finished = true;
                if (entry.phase.onFinished)
                    entry.phase.onFinished();
            }
        }
        if (m_elapsed >= m_duration) {
            m_running = false;
            if (onFinished)
                onFinished();
        }
        return changed || !m_running;
    }

    // Called once when every phase has run.
    std::function<void()> onFinished;

private:
    struct Entry {
        double startMs = 0.0;
        Phase phase;
        bool finished = false;
    };

    std::vector<Entry> m_entries;
    double m_cursorMs = 0.0;
    double m_anchorMs = 0.0;
    double m_duration = 0.0;
    double m_elapsed = 0.0;
    bool m_running = false;
};

} // namespace Lingmo::animation
