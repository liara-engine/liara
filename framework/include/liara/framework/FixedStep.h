/**
 * @file FixedStep.h
 * @brief A fixed simulation step driven by measured time, for a host's loop. ADR 0018 records why it is shaped so.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace Liara::Framework
{
    /** @brief How many fixed ticks a frame runs, and how far the simulation is into the next one. */
    struct FixedStepFrame
    {
        uint64_t m_Ticks = 0;  ///< How many times to advance the simulation by the step, this frame.
        float m_Alpha = 0.0F;  ///< Fraction of a step accumulated but not yet run, in [0, 1).
    };

    /**
     * @brief A fixed simulation step driven by measured time: Gaffer's accumulator, without the clock.
     *
     * It is pure. It never reads a clock: the caller measures the time elapsed since its last call and hands it to
     * `Advance`, which says how many ticks of exactly `StepSeconds()` that completes. The simulation is therefore
     * always advanced by the same step, whatever the frame rate.
     *
     * It is exact. A step such as 1/60 s is not a whole number of nanoseconds, so the step is never stored as one: the
     * accumulator counts nanoseconds times ticks per second, and one second, however it is cut into frames, completes
     * exactly as many ticks as the rate says. Nothing drifts over a long session.
     *
     * It is capped. An elapsed time above the cap counts as the cap, so a suspend, a breakpoint or a frame blocked for
     * seconds completes at most the cap's worth of ticks, and the simulation falls behind real time rather than
     * jumping. That also stops the spiral where each catch-up costs more than the time it catches up.
     *
     * `FixedStepFrame::m_Alpha` is how far the simulation is into the next tick, which is what interpolating between
     * the last two ticks will use.
     */
    class FixedStep
    {
    public:
        /** The default cap: a quarter of a second, which is 15 ticks at 60 Hz. */
        static constexpr uint64_t DEFAULT_MAX_ELAPSED_NS = 250'000'000;

        /**
         * @param ticksPerSecond The rate the simulation is advanced at.
         * @param maxElapsedNs The most one call to `Advance` accounts for; anything beyond it is dropped.
         *
         * @throws std::invalid_argument when either argument is 0, or when `maxElapsedNs` is so large that the
         *         accumulator would overflow: above `(UINT64_MAX - 1'000'000'000) / ticksPerSecond`. A step built as a
         *         `constexpr` from constants is checked by the compiler instead, and cannot throw at run time.
         */
        constexpr explicit FixedStep(const uint32_t ticksPerSecond,
                                     const uint64_t maxElapsedNs = DEFAULT_MAX_ELAPSED_NS)
            : m_TicksPerSecond(ticksPerSecond)
            , m_MaxElapsedNs(maxElapsedNs) {
            if (ticksPerSecond == 0) { throw std::invalid_argument("a fixed step needs at least one tick per second"); }
            if (maxElapsedNs == 0) { throw std::invalid_argument("a fixed step's cap must be longer than zero"); }
            if (maxElapsedNs > (std::numeric_limits<uint64_t>::max() - NS_PER_SECOND) / ticksPerSecond) {
                throw std::invalid_argument("a fixed step's cap is too large for its rate");
            }
        }

        /**
         * @brief Account for `elapsedNs` of real time, capped, and say how many ticks it completes.
         *
         * @param elapsedNs The time since the previous call, on a monotonic clock.
         */
        [[nodiscard]] FixedStepFrame Advance(const uint64_t elapsedNs) noexcept {
            m_Accumulated += std::min(elapsedNs, m_MaxElapsedNs) * m_TicksPerSecond;
            const uint64_t ticks = m_Accumulated / NS_PER_SECOND;
            m_Accumulated %= NS_PER_SECOND;
            // In double, then clamped: a remainder within a few parts per billion of a whole step rounds to 1 as a
            // float, and the factor is promised below 1.
            const auto alpha =
                static_cast<float>(static_cast<double>(m_Accumulated) / static_cast<double>(NS_PER_SECOND));
            return {.m_Ticks = ticks, .m_Alpha = std::min(alpha, std::nextafter(1.0F, 0.0F))};
        }

        /** @brief Real time left before the next tick is complete, rounded up: at least 1. */
        [[nodiscard]] constexpr uint64_t NsUntilNextTick() const noexcept {
            return (NS_PER_SECOND - m_Accumulated + m_TicksPerSecond - 1) / m_TicksPerSecond;
        }

        /** @brief The step, in seconds, as `liara_core_update` takes it. */
        [[nodiscard]] constexpr float StepSeconds() const noexcept {
            return 1.0F / static_cast<float>(m_TicksPerSecond);
        }

    private:
        static constexpr uint64_t NS_PER_SECOND = 1'000'000'000;

        uint64_t m_TicksPerSecond;
        uint64_t m_MaxElapsedNs;
        uint64_t m_Accumulated = 0;  ///< Nanoseconds times ticks per second, below NS_PER_SECOND between calls.
    };
}  // namespace Liara::Framework
