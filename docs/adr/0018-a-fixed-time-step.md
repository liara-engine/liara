---
title: "ADR 0018: A fixed time step, in the framework"
description: Why the host's loop advances the core by a fixed step from an exact, capped accumulator in the framework, and why interpolation is computed for but not yet used.
sidebar:
    label: "0018 · A fixed time step"
    order: 18
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-11-22 | Antoine  |

## Context

Until stage 6 of [v0.1](../../roadmap/v0-1/), the launcher measured how long each frame had taken, handed that delta to `liara_core_update`, and slept until a 60 Hz deadline before the next frame. That gives the simulation a step that changes with every frame, so the same input produces different results on a fast machine and a slow one, and a frame that stalls for a second advances the simulation by a second in one jump. Since stage 4 it also compounded with the swapchain: FIFO presentation already blocks until the display's refresh, and the sleep that followed it could land the loop on an irregular rate.

The fixed step was already placed. The boundary table of [liara-framework](../../modules/framework/) puts "Game clock: pause, time scale, fixed step" in the framework, as state that lives above the ABI and travels down as a delta, and the same page expected the game clock around v0.7. The decision to bring a fixed-step accumulator into v0.1 was taken on 2026-09-20, while planning ABI 1.0.0. What remained open was its shape, its home now, and how far it goes.

## Decision

**`Liara::Framework::FixedStep`, a header-only type in `framework/include/liara/framework/FixedStep.h`.** The launcher creates one at 60 ticks per second; each frame it hands it the time elapsed since the previous frame, runs `liara_core_update` as many times as it says with the same step each time, then extracts and submits one render packet.

**It is pure.** It never reads a clock. The caller measures the elapsed time on `liara_platform_time_now_ns` and passes it in, so the type holds local computation only, which is what the framework's third rule allows, and it is tested without any module.

**Its arithmetic is exact.** 1/60 s is not a whole number of nanoseconds, so the step is never stored as one. The accumulator counts nanoseconds times ticks per second in a `uint64_t`, a tick completes every 10⁹ of them, and the remainder carries to the next frame. One second cut into any frames completes exactly sixty ticks, and an hour exactly 216 000.

**It is capped at 250 ms per frame by default**, 15 ticks at 60 Hz. A longer frame counts as 250 ms and the rest is dropped. A slow frame under the cap is caught up in full, and the simulation never advances by more than one step at a time; past the cap, after a suspend or a breakpoint, the simulation falls behind real time rather than jumping. The cap also stops the spiral where each catch-up costs more than the time it catches up. Capping the elapsed time rather than the tick count keeps the cap in real time whatever the rate.

**The interpolation factor is computed and not yet used.** Each frame reports how far the accumulator is into the next tick, in [0, 1). Interpolating between the last two ticks needs something to interpolate, and the render packet has no transform until v0.2. The factor exists now so that interpolation, when it comes, adds one argument to the line that extracts the packet and changes nothing else in the loop.

## Alternatives considered

Five:

**The measured delta**, as the launcher did. Simple, and wrong for a simulation: the step varies with the frame rate, and a stalled frame becomes one enormous step.

**A semi-fixed step**, a measured delta split into steps no longer than a maximum. It bounds the step but keeps it variable, so the simulation stays non-deterministic across frame rates for the same cost as a fixed one.

**Interpolation now**, with the factor carried to the core as a parameter of `liara_core_get_render_packet`. The loop would reach its final shape at once, but the parameter would enter ABI 1.0.0 with nothing using it, for a packet that has no transform to interpolate and is replaced in 2.0.0.

**The accumulator in the launcher until v0.7**, a header local to `launcher/`. It leaves the framework untouched until the rest of the game clock exists, but the engine's only fixed step would be usable by no other host in the meantime and would move later anyway.

**No cap.** A resume from suspend would replay hours of ticks in one frame, and a machine slower than the simulation would never catch up.

## Consequences

The framework gains its first game-facing type and its first test suite, `framework/tests/`, which runs in CI. The rest of the game clock, pause, time scale and the formatting of a wall-clock timestamp, stays where [liara-framework](../../modules/framework/) puts it.

A host written in another language has no framework and writes its own accumulator. It is a dozen lines over `liara_platform_time_now_ns`, which is why the [boundary rule](../../modules/framework/#the-boundary-rule) keeps it out of the ABI.

When a frame runs several ticks, the core builds a render packet at every update and the host reads only the last. That cost is the v0.1 render packet experiment's to measure, and v0.2's extraction decides what to do about it.

## Revisit if

- The simulation needs a variable or adaptive rate, a physics step that changes with load for instance.
- Interpolation arrives and needs more from the loop than the factor.
- A measured frame regularly exceeds 250 ms on target hardware, so that the cap drops time in normal play rather than after a stall.
