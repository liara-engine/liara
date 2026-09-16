---
title: "liara-core"
description: The ECS, math, logger, settings, events and loop primitives. What is always there whatever else is loaded, and the practical test for what belongs.
sidebar:
  label: liara-core
  order: 3
---

The core implements everything that is shared, mandatory and not replaceable. It answers one question: what is always there, whatever modules are loaded?

It owns the data and the schedule, and the other modules transform data on a schedule it dictates. It is still a sibling of theirs rather than a runtime above them, and it reaches for none of them by name.

## Contents

**The ECS.** Entity allocation with generational handles, sparse-set component storage, the world container, the query API, system scheduling. Written by hand, for the reasons in [ADR 0009](../adr/0009-in-house-ecs/), and arriving in v0.2.

**The math layer.** Vector, matrix and quaternion types as plain C structs declared in `liara-interfaces`, with implementation functions operating on them. Internal computation may use GLM where convenient, and no GLM type ever crosses the boundary.

**The logger.** Structured entries, several sinks (stdout, a file, an in-memory ring buffer for the ImGui console), runtime level control, and thread safety. The design is what the previous engine prototyped, simplified. Whether it wraps spdlog or is written outright is still undecided and belongs to v0.2.

**The settings system.** Type-safe key-value storage serialized to TOML, with change notifications and category-based organization. Also carried over from the previous engine, with the format changed from a custom one to TOML.

**The event system.** Internal publish-subscribe for engine events (an entity created, a component added, an asset loaded) and for input events arriving from the platform module. It is decoupled from rendering on purpose: input flows through the core and may be consumed by scripting, the editor or game logic, not only by the renderer.

**The loop primitives.** `liara_core_update(core, dt)` advances the simulation by one tick, and `liara_core_get_render_packet()` hands back what the host should submit to the renderer.

:::caution[Three provisional entry points]
`liara_core_set_run_mode()`, `liara_core_run()` and `liara_core_stop()` currently let the core own the loop and call back into the host, which is how the Phase 0 demo runs. They are marked provisional in `core.h` and are removed in ABI 1.0.x. `LIARA_CORE_RUN_MODE_MANUAL` plus `liara_core_update()` is the arrangement [ADR 0003](../adr/0003-the-host-composes-modules/) actually describes, and it is what the test suite uses.
:::

**Module lifecycle.** Creation, destruction and the per-tick step, exposed through the C interface for a host to drive. The core neither loads nor registers nor holds a reference to another module. Deciding which renderer to pair it with, checking their versions against each other and wiring them together is the host's job.

## What it does not hold

No rendering. No window, no input device, no OS signal handling, which are `liara-platform`. No file loading or decoding, which is `liara-assets`. No audio device or mixing, which is `liara-audio`. No editor code, no gameplay, no tools.

The practical test is narrower than any list: the core does not open a file, does not talk to a device, and calls no OS API beyond threading and time. If it needs the outside world, it is not core.

## Internal layout

Internal and not part of any contract, so it may change without anyone being told. The shape it is heading for follows the categories above: `src/ecs/`, `src/math/`, `src/logger/`, `src/settings/`, `src/events/`, `src/loop/`. Today `src/` holds a single `core.cpp`, since none of those subsystems is written yet.
