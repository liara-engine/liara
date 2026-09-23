---
title: "ADR 0014: The platform's time facility"
description: Why the engine's clock is a monotonic nanosecond counter with a separate wall clock and a sleep that takes a deadline, and why none of the three entry points takes a handle.
sidebar:
    label: "0014 · The platform time facility"
    order: 14
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-09-21 | Antoine  |

## Context

`launcher/src/main.cpp` drove its loop off `std::chrono::steady_clock`, computed each frame delta as a `float` of seconds, and paced itself with `std::this_thread::sleep_for` against `constexpr float TARGET_FRAME_SECONDS = 1.0F / 60.0F`. That made the host's notion of time a property of whichever standard library compiled the host: the granularity `sleep_for` actually delivers differs between MSVC's standard library and libstdc++, and nothing in `main.cpp` recorded which one a given build got. A host written in Rust or Zig had no clock at all, and that host is not hypothetical: the `-runtime` presets resolve every entry point by name, so a module is reachable from any language with a C FFI (foreign function interface, the ability to call C symbols).

[liara-platform](../../modules/platform/) had owned timing on paper since Phase 0, in one sentence promising "a monotonic clock and high-resolution counters", and behind that sentence nothing was designed: no entry point, no unit, no statement of what the clock promises a caller. Designing one for the loop alone would have guaranteed that `std::chrono` came back somewhere else in the engine, so the facility is designed against every consumer that can be named today: the frame loop, frame-time measurement and profiling, animation, timeouts, and the audio module's scheduling.

## Decision

Four parts. The three entry points and what each one promises are described at [liara-platform](../../modules/platform/), and the contract clause by clause is the Doxygen of `liara/platform/platform.h` in `liara-interfaces`.

**A monotonic nanosecond counter is the primitive.** `liara_platform_time_now_ns` returns a `uint64_t` of nanoseconds from an unspecified origin, so only the difference between two readings is meaningful. Nanoseconds are the unit rather than a promise about granularity, which is why two readings taken close together may return the same value and a frame delta may legitimately be zero. At that unit a `uint64_t` covers about 584 years, so overflow gets no sentence in the header.

**The wall clock is a separate call, and its return type differs.** `liara_platform_time_wall_ns` returns an `int64_t` of nanoseconds since the Unix epoch, signed because 1970 is not the beginning of time and because the value steps backwards when NTP (the protocol that corrects a machine's clock against a time server) adjusts it. The differing type is doing work: passing a wall timestamp where a monotonic deadline belongs is at least a signed-to-unsigned conversion a reader can see. Nothing catches it mechanically today: the warning set in `LiaraCompilerSettings.cmake` runs from `-Wall -Wextra -Wpedantic` through `-Wshadow` and `-Wdouble-promotion`, and `-Wsign-conversion` is not in it.

**The sleep takes a deadline rather than a duration.** `liara_platform_time_sleep_until_ns` blocks until the monotonic counter has reached at least the `uint64_t` it is handed. The deadline form is the one that cannot be rebuilt from above: `clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME)` sleeps to an absolute point and drops nothing, while a caller computing `deadline - now` for itself has already lost whatever elapsed between reading the clock and issuing the call. The duration form is `sleep_until_ns(time_now_ns() + d)` and nothing more, so the [boundary rule](../../modules/framework/#the-boundary-rule) puts it in `Liara::Framework`.

**None of the three takes a handle.** `liara_platform_info` and `liara_platform_abi_version` were already handle-free, so the shape has precedent in the namespace, and three properties follow from it. The clock is callable before `liara_platform_create`, so a test or a profiler reads it without first creating an instance it does not otherwise want. There is no pointer to validate, so there is no failure to report: `time_now_ns` and `time_wall_ns` return their values directly and the sleep returns `void`, which is what `errors.md`, the error-reporting page of the interface guide, asks of a function that cannot fail. And no call can race another, because there is no instance for two threads to share. The symmetry argument for a handle is real, since `liara_platform_install_signal_handlers` and `liara_platform_quit_requested` do take one, and it loses because the handle would carry nothing: those two merge a process-global flag with per-instance window state from the next milestone on, while the clock has nothing to merge. An injectable clock or a time scale belongs to simulation time, which is the core's concern and the host's.

## Alternatives considered

Three, and the first two are what other projects ship.

**A frequency and a raw counter**, the shape of `QueryPerformanceFrequency` and `QueryPerformanceCounter` under Win32 and of `SDL_GetPerformanceFrequency` and `SDL_GetPerformanceCounter` in SDL. Rejected on two counts. It puts a Win32 implementation detail into a contract that has to hold on POSIX too, where `clock_gettime` already reports nanoseconds and the frequency is a constant nobody asked for. And it moves the same division into every caller, where each one gets its own opportunity to write it in `float` and lose the low bits.

**One fused clock**, a single entry point serving both the duration measurement and the log timestamp. Rejected because those are different questions: measuring a duration needs a value that never goes backwards, and timestamping a log line needs a value that matches a calendar. A fused clock makes one of the two wrong, and the consumer that picks the wrong one produces a negative frame delta the first time NTP steps the machine. Forcing the caller to name which clock it wants costs one word at the call site, and it is why `liara_platform_time_now_ns` and `liara_platform_time_wall_ns` are two names rather than one.

**Leaving time to the host's standard library**, which is what the launcher did until stage 1 of v0.1. Rejected because it makes pacing quality a property of the host's toolchain rather than of the engine, and because it leaves a host that is not written in C++ with nothing to call: `std::chrono` is not reachable from Rust or Zig, while under the `-runtime` presets `liara_platform_time_now_ns` is.

## Consequences

The duration form of the sleep lives above the contract rather than inside it. `sleep_ns` becomes the first game-facing function of `Liara::Framework`, by the boundary rule that [ADR 0013](../0013-the-framework-layer/) put in place. A game written in Rust or Zig does that addition itself, on top of `liara_platform_time_sleep_until_ns`.

A stop request is observed one frame late. `liara_platform_time_sleep_until_ns` resumes on the same absolute deadline when a signal interrupts it rather than returning early, so no caller has to wrap it in a loop against spurious wake-ups, and a caller that needs to react sooner than one frame slices its own wait. The price is that a Ctrl+C arriving during the pacing sleep is seen by the next call to `liara_platform_quit_requested`, about 16 ms later at 60 Hz.

The first implementation is plain `<chrono>` and `<thread>`: `steady_clock`, `system_clock` and `sleep_until`, with no `#ifdef` and no new dependency. C++20 guarantees that `system_clock`'s epoch is the Unix epoch, so the wall clock needs no platform-specific code to be exact. MSVC's sleep granularity is on the order of a millisecond, so Windows pacing is coarse until `clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME)` under POSIX and a high-resolution waitable timer under Win32 replace it. That replacement changes no line of `liara/platform/platform.h`.

## Revisit if

- A consumer asks what granularity the clock actually announces, which is the profiler's question when a 200 ns measurement has to be told apart from noise. The answer is an additive `liara_platform_time_resolution_ns`, and it is deliberately not shipped now as a no-op returning zero, because a caller cannot tell "unknown" from "one nanosecond" and a bare `uint64_t` has no `LIARA_RESULT_NOT_IMPLEMENTED` to say it with.
- A consumer depends on whether the monotonic counter advances across a system suspend, which the contract leaves unspecified. Pinning it down costs per-platform work for a guarantee no named consumer asks for, since Linux separates `CLOCK_MONOTONIC` from `CLOCK_BOOTTIME` and Windows offers no equivalent of `CLOCK_BOOTTIME`.
- A consumer needs the clock as a frequency reference rather than as a way to measure durations, which is where the audio module's sample scheduling is heading. The clause saying the tick rate is not promised to be constant starts to bite there: a sample-accurate schedule accumulates whatever the system's time discipline slews off `CLOCK_MONOTONIC`, which is up to a few hundred parts per million.
