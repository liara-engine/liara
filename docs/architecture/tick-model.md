---
title: Tick model and the application loop
description: Why the host owns the loop and the core only advances, and the provisional entry points that currently say otherwise.
sidebar:
  order: 6
---

The core does not own the application loop. It exposes a manual tick that advances the simulation by a given delta time, and whoever calls the core owns the loop itself.

In standalone mode the launcher implements it: poll the platform for input and shutdown requests, advance the core by one tick, extract the render packet and the audio events, submit them to the renderer and to the audio module, present, repeat.

The host owns the loop because the host is the only participant that knows every module present. A module running the loop would have to know its siblings, which is exactly the coupling the whole architecture exists to prevent, and [ADR 0003](../../adr/0003-the-host-composes-modules/) is where that is argued.

:::caution[Not true yet]
`liara-core` currently exposes `liara_core_set_run_mode()`, `liara_core_run()` and `liara_core_stop()`, so the Phase 0 demo runs its loop inside the core and calls back into the launcher through a late update callback. The three are marked provisional in `core.h` and are removed in ABI 1.0.x.

`LIARA_CORE_RUN_MODE_MANUAL` plus `liara_core_update()` is the arrangement this page describes, it exists today, and it is what the `liara-core` test suite uses.
:::
