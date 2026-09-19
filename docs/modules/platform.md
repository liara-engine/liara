---
title: "liara-platform"
description: Everything the engine needs from the operating system behind one interface, and the poll-only shutdown design that keeps signal handling safe.
sidebar:
  label: liara-platform
  order: 4
---

Its capabilities arrive with v0.1; the repository exists from Phase 0, holding the module's self-description and nothing else. Everything the engine needs from the operating system, behind one interface, which is what makes "Linux and Windows are both first-class" a property of one repository instead of an `#ifdef` scattered through all of them.

## Contents

**Window management.** Creation, resizing, fullscreen, and exposure of the native handle the renderer needs to create its surface. SDL3 is the backend, and no SDL type crosses the module boundary.

**Input devices.** Keyboard, mouse, gamepad. The module reports physical device state and physical events, and it does not know what an action is. Mapping a physical input to a logical action is a consumer's concern.

**OS signals and shutdown requests.** SIGINT, SIGTERM, the Windows console control events, and the window's close button. These are deliberately one thing at the interface, because they are all "the user asked the process to stop", and a host that handles one handles all of them.

**Timing.** A monotonic clock and high-resolution counters, so that the loop's notion of time does not depend on which standard library the host happened to be built against.

## Shutdown is poll-only

The interface exposes shutdown as a flag the host polls, never as a callback:

```c
liara_result_t liara_platform_install_signal_handlers(liara_platform_handle_t*);
bool           liara_platform_quit_requested(const liara_platform_handle_t*);
```

Three constraints follow from that shape, and all three are easy to violate later.

**No function pointer crosses the boundary for this.** A callback invoked from a POSIX signal handler would be async-signal-unsafe, and a callback stored in a struct is one of the [anti-patterns](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/anti-patterns/) the interface guide lists. The handler does the only thing it is allowed to do, which is write a flag, and the loop reads it.

**Signal handlers are process-global rather than per-instance.** `liara_platform_install_signal_handlers` is documented as idempotent and installed at most once per process, whatever number of platform handles exist.

**It works without a window.** Installing the handlers does not require the windowing backend to be initialized, so a headless tool (a test harness, a future asset cooker) can use the module for shutdown handling alone.

## What it does not hold

No ECS, no rendering, and no audio device. Audio talks to the OS too, and the boundary here is the concern rather than the fact of being OS-specific.

No file I/O beyond resolving standard paths, since reading files is `liara-assets`. No logical input mapping.

## Replaceability

Replaceable, and meant to be. A platform module built on GLFW, on winit, or directly on Wayland and Win32 is a legitimate substitute, and confining the SDL3 dependency here is precisely what keeps that true.
