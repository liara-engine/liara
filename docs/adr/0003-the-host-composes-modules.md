---
title: "ADR 0003: The host composes modules"
description: Why the launcher creates, checks and wires every module, why the core does not, and what the -runtime presets prove about it.
sidebar:
    label: "0003 · Host composes modules"
    order: 3
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-08-26 | Antoine  |

## Context

Liara is a set of modules that version on their own cadence and talk to each other through a C ABI (the headers in `liara-interfaces`, which is the only dependency a module is allowed to have). Something still has to decide which of them exist in a given run, create them, and connect them together.

The core is the obvious candidate. It is the module everything else seems to revolve around, it is created first, and handing it the job costs no new component.

That instinct had already been acted on before any of this was written down. The core held a reference to the renderer, drove the frame loop, and produced a render packet (the flat list of plain-old-data structures the core builds once per tick, described in `ARCHITECTURE.md` §6.3<!-- TODO link → the render packet page, once architecture is split into a directory -->) that the renderer consumed.

Read from outside it, the same arrangement costs three things at once: the core can no longer be built without a renderer, nor tested without one, and its release cadence stops being its own. Which is exactly the independence the multi-repository layout of [ADR 0001](../0001-multi-repository-layout/) was set up to buy.

And in a static build, nothing complains about any of it. The symbols resolve at link time, the binary runs, so the coupling accumulates quietly and surfaces only the day somebody tries to load a module at runtime, by which point it is everywhere.

## Decision

The host composes. The launcher today, and the editor later, creates each module on its own, checks the ABI version each one reports, wires them together, and owns the frame loop.

A module never creates, loads, references or links against a sibling. Its only dependency is `liara-interfaces`. Where two modules have to exchange data, they exchange a type declared in the contract and the host carries it across: the renderer receives a render packet, it does not ask the core for one.

:::caution[The frame loop is not there yet]
`liara-core` still exposes `liara_core_set_run_mode()`, `liara_core_run()` and `liara_core_stop()`, so the Phase 0 demo runs its loop inside the core and calls back into the launcher through a late update callback. Those three entry points are marked provisional in `core.h` and are removed in ABI 1.0.x. The shape this decision actually describes is `LIARA_CORE_RUN_MODE_MANUAL` plus `liara_core_update()`, which exists today and is what the `liara-core` test suite uses. It's an artifact of the bootstrapping phase, when the launcher was not created yet, but a loop was needed to test the core.
:::

## Alternatives considered

Three, all rejected.

**The core orchestrates.** Fewer moving parts, and the host shrinks to a thin `main()`. Rejected because it makes `liara-core` depend on every module it orchestrates, which is the dependency graph ADR 0001 exists to avoid. It also gives "core" two unrelated meanings at once, a set of foundational services and an application framework, and once the word covers both there is no test left for whether something belongs there.

**A dedicated orchestration module**, sitting between the host and the modules. Rejected as premature: it is the previous option with one more name on it, and it would have to be composed by something anyway. If the launcher and the editor eventually share enough composition code to justify pulling it out, that extraction will be a better informed decision than this one.

**A plugin registry**, where modules register themselves at load time. Rejected for now: it solves discovery, which is not a problem yet, and it solves it by making the set of loaded modules a runtime property that no build-time check can verify. Worth reopening the day a host has to load a module it was not compiled to know about.

## Consequences

The host is the only component that knows the module graph, and it grows as modules are added. That is accepted. Composition logic has to live somewhere, and one place allowed to know everything is easier to audit than several places that each know slightly too much.

Two modules cannot call each other, even when it would be convenient. The data goes through a type in the contract and through the host, so adding an interaction between two modules means changing `liara-interfaces`. That is deliberately more expensive than adding a function call, because it is a change to the contract everything else depends on.

Testing a module on its own becomes cheap, since there is no sibling to stand up first. The `liara-core` suite creates a core, switches it to `LIARA_CORE_RUN_MODE_MANUAL`, ticks it once with a delta of 1/60 s and reads the packet back, with no renderer anywhere in the test binary.

The `-runtime` presets are what keep the decision honest. Under them the launcher links against `liara-interfaces` and pulls in `Liara::Core` and `Liara::Renderer` with `$<COMPILE_ONLY:>` (headers, no link), then opens both shared libraries with `dlopen` on Linux and `LoadLibrary` on Windows, resolves `liara_core_info` and `liara_renderer_info` by name, and refuses to proceed when what they report is incompatible with `MIN_ABI_VERSION`, currently 0.2.0. What that proves today is that the launcher itself carries no link-time dependency on a module. Whether a module that picked one up on a sibling would fail there rather than somewhere later has not been tested, so that half of the claim is still an expectation.

## Revisit if

- A host needs to load modules it was not compiled to know about, which turns composition into discovery and makes the plugin-registry alternative worth reopening.
- The launcher and the editor accumulate enough shared composition logic that duplicating it costs more than extracting it.
- Passing data through the contract becomes the bottleneck for an interaction that is genuinely hot, with a measurement to show it.
