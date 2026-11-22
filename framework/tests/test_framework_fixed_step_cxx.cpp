// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/framework/FixedStep.h>

#include <cstdint>
#include <stdexcept>

#include <doctest/doctest.h>

// The fixed step is pure: every case hands it elapsed times and reads what it says, with no clock and no module.

using Step = Liara::Framework::FixedStep;
using Frame = Liara::Framework::FixedStepFrame;

namespace
{
    constexpr uint64_t QUARTER_SECOND_NS = 250'000'000;
    constexpr uint64_t TEN_SECONDS_NS = 10'000'000'000;

    /** One second at 60 Hz in sixty frames, none of them a whole step: twenty short ones, then forty long ones. */
    uint64_t RunExactSecond(Step& step, Frame& last) {
        uint64_t ticks = 0;
        for (int frame = 0; frame < 20; ++frame) {
            last = step.Advance(16'666'666);
            ticks += last.m_Ticks;
        }
        for (int frame = 0; frame < 40; ++frame) {
            last = step.Advance(16'666'667);
            ticks += last.m_Ticks;
        }
        return ticks;
    }
}  // namespace

TEST_CASE("FixedStep - nothing elapsed, nothing ticks") {
    Step step(60);
    const Frame frame = step.Advance(0);
    CHECK(frame.m_Ticks == 0);
    CHECK(frame.m_Alpha == 0.0F);
}

TEST_CASE("FixedStep - less than a step keeps the remainder") {
    Step step(50);  // 20 ms
    const Frame frame = step.Advance(5'000'000);
    CHECK(frame.m_Ticks == 0);
    CHECK(frame.m_Alpha == doctest::Approx(0.25));
}

TEST_CASE("FixedStep - exactly one step") {
    Step step(50);
    const Frame frame = step.Advance(20'000'000);
    CHECK(frame.m_Ticks == 1);
    CHECK(frame.m_Alpha == 0.0F);
}

TEST_CASE("FixedStep - several steps at once") {
    Step step(50);
    const Frame frame = step.Advance(70'000'000);
    CHECK(frame.m_Ticks == 3);
    CHECK(frame.m_Alpha == doctest::Approx(0.5));
}

TEST_CASE("FixedStep - the remainder carries across frames") {
    Step step(50);
    CHECK(step.Advance(10'000'000).m_Ticks == 0);
    CHECK(step.Advance(10'000'000).m_Ticks == 1);
    CHECK(step.Advance(10'000'000).m_Ticks == 0);
    CHECK(step.Advance(10'000'000).m_Ticks == 1);
}

// 1/60 s is no whole number of nanoseconds, which is what these four cases are about.

TEST_CASE("FixedStep - no drift at 60 Hz: four quarters of a second") {
    Step step(60);
    uint64_t ticks = 0;
    Frame last;
    for (int frame = 0; frame < 4; ++frame) {
        last = step.Advance(QUARTER_SECOND_NS);
        ticks += last.m_Ticks;
    }
    CHECK(ticks == 60);
    CHECK(last.m_Alpha == 0.0F);
}

TEST_CASE("FixedStep - no drift at 60 Hz: sixty frames summing to one second") {
    Step step(60);
    Frame last;
    CHECK(RunExactSecond(step, last) == 60);
    CHECK(last.m_Alpha == 0.0F);
}

TEST_CASE("FixedStep - no drift at 60 Hz: sixty frames 40 ns short of a second") {
    Step step(60);
    uint64_t ticks = 0;
    for (int frame = 0; frame < 60; ++frame) { ticks += step.Advance(16'666'666).m_Ticks; }
    CHECK(ticks == 59);
}

TEST_CASE("FixedStep - no drift at 60 Hz: an hour") {
    Step step(60);
    uint64_t ticks = 0;
    Frame last;
    for (int second = 0; second < 3600; ++second) { ticks += RunExactSecond(step, last); }
    CHECK(ticks == 216'000);
    CHECK(last.m_Alpha == 0.0F);
}

TEST_CASE("FixedStep - alpha stays in [0, 1)") {
    Step step(60);
    uint64_t state = 0x2545F4914F6CDD1DULL;
    bool allInRange = true;
    for (int frame = 0; frame < 10'000; ++frame) {
        // Knuth's MMIX linear congruential generator: a fixed, reproducible sequence of elapsed times.
        state = (state * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
        const Frame result = step.Advance((state >> 16U) % (QUARTER_SECOND_NS + 1));
        allInRange = allInRange && result.m_Alpha >= 0.0F && result.m_Alpha < 1.0F;
    }
    CHECK(allInRange);
}

TEST_CASE("FixedStep - alpha never reaches 1") {
    // A remainder of 999 999 999 out of 10^9 is 0.999999999, which a float rounds to exactly 1.
    Step step(1);
    (void)step.Advance(249'999'999);
    (void)step.Advance(QUARTER_SECOND_NS);
    (void)step.Advance(QUARTER_SECOND_NS);
    const Frame frame = step.Advance(QUARTER_SECOND_NS);
    CHECK(frame.m_Ticks == 0);
    CHECK(frame.m_Alpha < 1.0F);
}

TEST_CASE("FixedStep - the cap drops what lies beyond it") {
    Step capped(60);
    const Frame frame = capped.Advance(TEN_SECONDS_NS);
    CHECK(frame.m_Ticks == 15);
    CHECK(frame.m_Alpha == 0.0F);

    Step custom(60, 100'000'000);
    CHECK(custom.Advance(TEN_SECONDS_NS).m_Ticks == 6);
}

TEST_CASE("FixedStep - a step longer than the cap still ticks") {
    Step step(1);
    CHECK(step.Advance(TEN_SECONDS_NS).m_Ticks == 0);
    CHECK(step.Advance(TEN_SECONDS_NS).m_Ticks == 0);
    CHECK(step.Advance(TEN_SECONDS_NS).m_Ticks == 0);
    const Frame fourth = step.Advance(TEN_SECONDS_NS);
    CHECK(fourth.m_Ticks == 1);
    CHECK(fourth.m_Alpha == 0.0F);
}

TEST_CASE("FixedStep - time until the next tick") {
    CHECK(Step(60).NsUntilNextTick() == 16'666'667);

    Step partial(50);
    (void)partial.Advance(10'000'000);
    CHECK(partial.NsUntilNextTick() == 10'000'000);

    Step almost(60);
    (void)almost.Advance(16'666'666);
    CHECK(almost.NsUntilNextTick() == 1);

    Step whole(50);
    (void)whole.Advance(20'000'000);
    CHECK(whole.NsUntilNextTick() == 20'000'000);
}

TEST_CASE("FixedStep - the step in seconds") { CHECK(Step(60).StepSeconds() == 1.0F / 60.0F); }

TEST_CASE("FixedStep - refuses what has no meaning") {
    CHECK_THROWS_AS(Step(0), std::invalid_argument);
    CHECK_THROWS_AS(Step(60, 0), std::invalid_argument);
    CHECK_THROWS_AS(Step(1'000'000, UINT64_MAX / 2), std::invalid_argument);
}

// NOLINTEND(readability-identifier-naming)
