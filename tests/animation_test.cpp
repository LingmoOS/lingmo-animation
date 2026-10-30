// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause
//
// No test framework is needed: the primitives are pure arithmetic, so the
// checks run with the platform, in a package build as well as locally.

#include <cmath>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

#include "lingmo/animation/animation.h"

using namespace Lingmo::animation;

namespace {

int failures = 0;
int checks = 0;

void check(bool passed, const char *what, const char *file, int line)
{
    ++checks;
    if (passed)
        return;
    ++failures;
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, what);
}

#define EXPECT_TRUE(condition) check((condition), #condition, __FILE__, __LINE__)
#define EXPECT_FALSE(condition) check(!(condition), "!(" #condition ")", __FILE__, __LINE__)
#define EXPECT_NEAR(actual, expected, tolerance) \
    check(std::abs((actual) - (expected)) <= (tolerance), #actual " ~= " #expected, __FILE__, __LINE__)
#define EXPECT_EQ(actual, expected) \
    check((actual) == (expected), #actual " == " #expected, __FILE__, __LINE__)

void testCurves()
{
    const Curve curves[] = {Curve::Linear,      Curve::InQuad,     Curve::OutQuad,
                            Curve::InOutQuad,   Curve::InCubic,    Curve::OutCubic,
                            Curve::InOutCubic,  Curve::InQuart,    Curve::OutQuart,
                            Curve::InOutQuart,  Curve::OutQuint,   Curve::OutSine,
                            Curve::InOutSine,   Curve::OutBack};
    for (Curve curve : curves) {
        EXPECT_NEAR(evaluate(curve, 0.0), 0.0, 1e-9);
        EXPECT_NEAR(evaluate(curve, 1.0), 1.0, 1e-9);
        // Clamped outside the unit interval.
        EXPECT_NEAR(evaluate(curve, -1.0), 0.0, 1e-9);
        EXPECT_NEAR(evaluate(curve, 2.0), 1.0, 1e-9);
    }

    EXPECT_NEAR(evaluate(Curve::Linear, 0.25), 0.25, 1e-9);
    EXPECT_NEAR(evaluate(Curve::InOutCubic, 0.5), 0.5, 1e-9);
    // Out eases: it is past the halfway mark halfway through.
    EXPECT_TRUE(evaluate(Curve::OutQuint, 0.5) > 0.5);
    EXPECT_TRUE(evaluate(Curve::InQuad, 0.5) < 0.5);
    // OutBack overshoots before settling.
    EXPECT_TRUE(evaluate(Curve::OutBack, 0.7) > 1.0);

    EXPECT_NEAR(lerp(2.0, 10.0, 0.25), 4.0, 1e-9);
}

void testTween()
{
    Tween tween(0.0, 100.0, Curve::Linear);
    int finished = 0;
    tween.onFinished = [&finished] { ++finished; };
    EXPECT_FALSE(tween.running());

    tween.setTarget(10.0);
    EXPECT_TRUE(tween.running());
    EXPECT_TRUE(tween.advance(50.0));
    EXPECT_NEAR(tween.value(), 5.0, 1e-9);
    EXPECT_NEAR(tween.progress(), 0.5, 1e-9);
    tween.advance(50.0);
    EXPECT_NEAR(tween.value(), 10.0, 1e-9);
    EXPECT_FALSE(tween.running());
    EXPECT_EQ(finished, 1);
    tween.advance(50.0);
    EXPECT_EQ(finished, 1);

    // A retarget keeps the value continuous and travels from there.
    Tween retarget(0.0, 100.0, Curve::Linear);
    retarget.setTarget(10.0);
    retarget.advance(25.0);
    EXPECT_NEAR(retarget.value(), 2.5, 1e-9);
    retarget.setTarget(20.0);
    EXPECT_NEAR(retarget.value(), 2.5, 1e-9);
    retarget.advance(25.0);
    EXPECT_NEAR(retarget.value(), 2.5 + 17.5 * 0.25, 1e-9);

    // How often it is stepped does not change where it lands.
    Tween many(0.0, 100.0, Curve::Linear);
    many.setTarget(10.0);
    for (int i = 0; i < 10; ++i)
        many.advance(10.0);
    EXPECT_NEAR(many.value(), 10.0, 1e-9);
    EXPECT_FALSE(many.running());

    // A zero duration is a jump.
    Tween jump(0.0, 0.0);
    jump.setTarget(5.0);
    EXPECT_NEAR(jump.value(), 5.0, 1e-9);
    EXPECT_FALSE(jump.running());

    // stop() freezes, finish() jumps.
    Tween stopped(0.0, 100.0, Curve::Linear);
    stopped.setTarget(10.0);
    stopped.advance(50.0);
    stopped.stop();
    EXPECT_NEAR(stopped.value(), 5.0, 1e-9);
    EXPECT_FALSE(stopped.running());
    stopped.setTarget(10.0);
    stopped.finish();
    EXPECT_NEAR(stopped.value(), 10.0, 1e-9);

    // A custom curve is honoured.
    Tween custom(0.0, 100.0);
    custom.setCurve(Curve::OutCubic);
    custom.setTarget(1.0);
    custom.advance(50.0);
    EXPECT_NEAR(custom.value(), evaluate(Curve::OutCubic, 0.5), 1e-9);
}

void testSmoothing()
{
    Smoothing smoothing(0.0, 100.0);
    int finished = 0;
    smoothing.onFinished = [&finished] { ++finished; };
    smoothing.setTarget(10.0);
    EXPECT_TRUE(smoothing.running());
    smoothing.advance(100.0);
    EXPECT_NEAR(smoothing.value(), 10.0 * (1.0 - std::exp(-1.0)), 1e-6);
    // Enough time snaps to the target and stops.
    smoothing.advance(10'000.0);
    EXPECT_NEAR(smoothing.value(), 10.0, 1e-9);
    EXPECT_FALSE(smoothing.running());
    EXPECT_EQ(finished, 1);

    // Retargeting does not move the value.
    Smoothing retarget(0.0, 100.0);
    retarget.setTarget(1.0);
    retarget.advance(50.0);
    const double before = retarget.value();
    retarget.setTarget(0.0);
    EXPECT_NEAR(retarget.value(), before, 1e-12);
    retarget.advance(10'000.0);
    EXPECT_NEAR(retarget.value(), 0.0, 1e-9);
    EXPECT_FALSE(retarget.running());

    // No time constant means it is a jump.
    Smoothing jump(0.0, 0.0);
    jump.setTarget(5.0);
    EXPECT_NEAR(jump.value(), 5.0, 1e-9);
    EXPECT_FALSE(jump.running());
}

void testSpring()
{
    Spring spring(0.0);
    int finished = 0;
    spring.onFinished = [&finished] { ++finished; };
    spring.setTarget(10.0);
    EXPECT_TRUE(spring.running());
    for (int i = 0; i < 240 && spring.running(); ++i)
        spring.advance(1000.0 / 60.0);
    EXPECT_FALSE(spring.running());
    EXPECT_NEAR(spring.value(), 10.0, 1e-9);
    EXPECT_NEAR(spring.velocity(), 0.0, 1e-9);
    EXPECT_EQ(finished, 1);

    // A single very long step is broken into substeps and still settles.
    Spring longStep(0.0);
    longStep.setTarget(10.0);
    longStep.advance(5'000.0);
    EXPECT_TRUE(std::isfinite(longStep.value()));
    longStep.advance(5'000.0);
    EXPECT_FALSE(longStep.running());
    EXPECT_NEAR(longStep.value(), 10.0, 1e-9);

    // Retargeting keeps the value and the velocity.
    Spring retarget(0.0);
    retarget.setTarget(10.0);
    for (int i = 0; i < 10; ++i)
        retarget.advance(1000.0 / 60.0);
    const double before = retarget.value();
    const double velocity = retarget.velocity();
    retarget.setTarget(-5.0);
    EXPECT_NEAR(retarget.value(), before, 1e-12);
    EXPECT_NEAR(retarget.velocity(), velocity, 1e-12);
}

void testHop()
{
    Hop hop;
    int finished = 0;
    hop.onFinished = [&finished] { ++finished; };
    EXPECT_NEAR(hop.lift(), 0.0, 1e-12);

    hop.start(100.0);
    EXPECT_TRUE(hop.running());
    EXPECT_NEAR(hop.lift(), 0.0, 1e-9);
    hop.advance(150.0);
    const double firstPeak = 18.0 * std::exp(-1.1 * 150.0 / 900.0);
    EXPECT_NEAR(hop.lift(), firstPeak, 1e-6);
    EXPECT_TRUE(hop.lift() > 1.0);

    // On the ground at the end of the first hop, then a lower second one.
    hop.advance(150.0);
    EXPECT_NEAR(hop.lift(), 0.0, 1e-9);
    hop.advance(150.0);
    EXPECT_TRUE(hop.lift() > 1.0);
    EXPECT_TRUE(hop.lift() < firstPeak);

    hop.advance(450.0);
    EXPECT_FALSE(hop.running());
    EXPECT_NEAR(hop.lift(), 0.0, 1e-9);
    EXPECT_EQ(finished, 1);

    // A silent hop is no hop.
    Hop silent;
    silent.startWithAmplitude(0.0);
    EXPECT_FALSE(silent.running());

    // stop() lands at once.
    Hop stopped;
    stopped.start(100.0);
    stopped.advance(100.0);
    stopped.stop();
    EXPECT_FALSE(stopped.running());
    EXPECT_NEAR(stopped.lift(), 0.0, 1e-9);
}

void testStepper()
{
    Stepper stepper(100.0);
    EXPECT_NEAR(stepper.tick(1'000'000'000), 0.0, 1e-12);
    EXPECT_NEAR(stepper.tick(1'016'000'000), 16.0, 1e-9);
    EXPECT_NEAR(stepper.tick(1'016'000'000), 0.0, 1e-12);
    // A stall is clamped.
    EXPECT_NEAR(stepper.tick(1'216'000'000), 100.0, 1e-9);
    stepper.reset();
    EXPECT_NEAR(stepper.tick(5'000'000'000), 0.0, 1e-12);
}

void testTimeline()
{
    std::vector<std::string> events;
    Timeline timeline;
    timeline
            .then({100.0, Curve::Linear,
                   [&events](double progress) {
                       events.push_back("a" + std::to_string(int(std::lround(progress * 100.0))));
                   },
                   [&events] { events.push_back("a-done"); }})
            .then({100.0, Curve::Linear,
                   [&events](double progress) {
                       events.push_back("b" + std::to_string(int(std::lround(progress * 100.0))));
                   },
                   [&events] { events.push_back("b-done"); }});
    EXPECT_NEAR(timeline.durationMs(), 200.0, 1e-9);
    bool done = false;
    timeline.onFinished = [&done] { done = true; };

    timeline.start();
    EXPECT_TRUE(timeline.running());
    timeline.advance(50.0);
    EXPECT_EQ(events.back(), "a50");
    timeline.advance(50.0);
    EXPECT_EQ(events.back(), "b0");
    EXPECT_TRUE(events.size() >= 3 && events[1] == "a100" && events[2] == "a-done");
    timeline.advance(100.0);
    EXPECT_EQ(events.back(), "b-done");
    EXPECT_FALSE(timeline.running());
    EXPECT_TRUE(done);
    EXPECT_NEAR(timeline.elapsedMs(), 200.0, 1e-9);

    // together() runs a phase beside the one before it.
    Timeline parallel;
    parallel
            .then({100.0, Curve::Linear, [](double) {}, nullptr})
            .together({100.0, Curve::Linear, [](double) {}, nullptr});
    EXPECT_NEAR(parallel.durationMs(), 100.0, 1e-9);
    bool parallelDone = false;
    parallel.onFinished = [&parallelDone] { parallelDone = true; };
    parallel.start();
    parallel.advance(100.0);
    EXPECT_FALSE(parallel.running());
    EXPECT_TRUE(parallelDone);

    // An empty timeline is not running.
    Timeline empty;
    empty.start();
    EXPECT_FALSE(empty.running());
}

void testAdvanceable()
{
    // The primitives share one interface, so a driver can hold any of them.
    Tween tween(0.0, 100.0, Curve::Linear);
    Smoothing smoothing;
    Spring spring;
    Hop hop;
    Timeline timeline;
    std::vector<Advanceable *> motions = {&tween, &smoothing, &spring, &hop, &timeline};
    EXPECT_EQ(motions.size(), 5u);
    for (Advanceable *motion : motions)
        EXPECT_FALSE(motion->running());
}

} // namespace

int main()
{
    testCurves();
    testTween();
    testSmoothing();
    testSpring();
    testHop();
    testStepper();
    testTimeline();
    testAdvanceable();

    if (failures == 0) {
        std::printf("ok: %d checks\n", checks);
        return 0;
    }
    std::fprintf(stderr, "%d of %d checks failed\n", failures, checks);
    return 1;
}
