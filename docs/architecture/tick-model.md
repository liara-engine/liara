---
title: Tick model and the application loop
description: Why the host owns the loop and the core only advances, and what that buys a headless tool and an editor.
sidebar:
  order: 6
---

The core does not own the application loop. It exposes a manual tick that advances the simulation by a given delta time, and whoever calls the core owns the loop itself.

In standalone mode the launcher implements it: poll the platform for input and shutdown requests, advance the core by one tick, extract the render packet and the audio events, submit them to the renderer and to the audio module, present, repeat.

The host owns the loop because the host is the only participant that knows every module present. A module running the loop would have to know its siblings, which is exactly the coupling the whole architecture exists to prevent, and [ADR 0003](../../adr/0003-the-host-composes-modules/) is where that is argued.

The core's entire loop surface is therefore `liara_core_update(core, delta_time)` and `liara_core_get_render_packet()`. There is no run mode to select and no callback to register, which is what makes a headless test or an editor stepping one frame at a time ordinary rather than a special mode.

Until v0.1 the core also exposed `liara_core_set_run_mode()`, `liara_core_run()` and `liara_core_stop()`, which let it own the loop and call back into the launcher. They were an artifact of the bootstrapping phase, when there was no launcher yet but the core still had to be exercised, and they were removed on the way to ABI 1.0.0.
