---
title: "liara-platform"
description: Everything the engine needs from the operating system behind one interface, the poll-only shutdown design that keeps signal handling safe, and the three time entry points the loop runs on.
sidebar:
  label: liara-platform
  order: 4
---

Its first capabilities arrive with v0.1: the create and destroy lifecycle, the shutdown surface, the clock, and windows with their events are built, while the input devices are designed and not yet written. The repository exists from Phase 0, when it held the module's self-description and nothing else. Everything the engine needs from the operating system, behind one interface, which is what makes "Linux and Windows are both first-class" a property of one repository instead of an `#ifdef` scattered through all of them.

## Contents

**Window management.** A window is a handle of its own, `liara_platform_window_handle_t`, created from a platform instance with `liara_platform_window_create` and destroyed with `liara_platform_window_destroy` before the instance is. SDL3 is the backend, and no SDL type crosses the module boundary. The size a caller asks for is in the window system's logical units and is a request, which a tiling window manager ignores; every size the module reports back is in pixels, which is what a swapchain needs, and the field names (`requested_size`, `pixel_size`) carry the unit. `liara_platform_window_get_native` fills the `liara_native_window_t` a renderer creates its surface from, declared in the shared `liara/native_window.h` because a window can come from somewhere other than this module; [ADR 0015](../../adr/0015-the-window-and-its-native-handle/) records why it is a tagged union. A window's content is unspecified until something presents to it, and until the renderer does, the module paints a solid placeholder, since Wayland never shows a surface nothing was committed to. A window is always resizable. Every window and event entry point is called from the process's main thread, which SDL requires on several platforms.

**Input devices.** Keyboard, mouse, gamepad. The module reports physical device state and physical events, and it does not know what an action is. Mapping a physical input to a logical action is a consumer's concern.

**OS signals and shutdown requests.** SIGINT, SIGTERM, the Windows console control events, and the window's close button. These are deliberately one thing at the interface, because they are all "the user asked the process to stop", and a host that handles one handles all of them.

**Timing.** Three entry points, none of which takes a handle. `liara_platform_time_now_ns` reads a monotonic counter in nanoseconds from an unspecified origin, so only the difference between two readings means anything, and the value never decreases as seen from any thread. `liara_platform_time_wall_ns` reads the wall clock as a signed count of nanoseconds since the Unix epoch, UTC as the operating system reports it, signed because the value steps backwards when NTP (the protocol that corrects a machine's clock against a time server) adjusts it. `liara_platform_time_sleep_until_ns` blocks until the monotonic counter has reached a deadline, at least and never less, with no bound on the overshoot and no early return when a signal arrives. The two clocks are separate calls because they answer different questions, and fusing them is the classic bug. The loop's notion of time therefore stops depending on which standard library the host happened to be built against, and the contract stops a consumer picking the wrong clock by making it name the one it wants: `liara_platform_time_now_ns` to measure a duration, `liara_platform_time_wall_ns` to timestamp a line.

The duration form of the sleep is absent on purpose. It is `sleep_until_ns(time_now_ns() + d)` and nothing more, so the [boundary rule](../framework/#the-boundary-rule) puts it in `Liara::Framework`. The alternatives that lost, including the frequency-and-raw-counter pair that `QueryPerformanceFrequency` and `QueryPerformanceCounter` expose, are in [ADR 0014](../../adr/0014-the-platform-time-facility/).

## Events

`liara_platform_poll_event` returns one event per call into a `liara_platform_event_t` the caller allocates, 64 bytes on 64-bit targets, and `false` once the drain is over. A drain is the run of calls up to one returning `false`: its first call pumps the operating system once and the others only read what is already queued for that instance, so a `while` loop over `liara_platform_poll_event` on one instance terminates under a continuous stream of events such as a resize drag. Each event carries its type, the window it concerns, and a `timestamp_ns` on the `liara_platform_time_now_ns` clock, read when the module translated the event. Two types exist in v0.1: `LIARA_PLATFORM_EVENT_WINDOW_CLOSE_REQUESTED`, the user asking a window to close, which does not destroy it, and `LIARA_PLATFORM_EVENT_WINDOW_RESIZED`, a change of size in pixels. New types arrive in minor versions with their payload inside the record's padding, so a host skips any type it does not recognise. An instance sees the events of its own windows only, and destroying a window discards those still queued for it.

## Shutdown is poll-only

The interface exposes shutdown as a flag the host polls, never as a callback:

```c
liara_result_t liara_platform_install_signal_handlers(liara_platform_handle_t*);
bool           liara_platform_quit_requested(const liara_platform_handle_t*);
```

A close request joins the flag when it concerns the last open window. A close request on the last open window of an instance (created and not yet destroyed) raises that instance's flag on top of producing its event, so with one window, closing it is exactly a Ctrl+C; with several, closing one produces the event alone and the host decides. A quit the window system itself requests, such as a session ending, raises the process-global flag the signal handler writes. Both reach the flag through `liara_platform_poll_event`, which is why a host polls its events before it reads `liara_platform_quit_requested`.

Three constraints follow from that shape, and all three are easy to violate later.

**No function pointer crosses the boundary for this.** A callback invoked from a POSIX signal handler would be async-signal-unsafe, and a callback stored in a struct is one of the [anti-patterns](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/anti-patterns/) the interface guide lists. The handler does the only thing it is allowed to do, which is write a flag, and the loop reads it.

**Signal handlers are process-global rather than per-instance.** `liara_platform_install_signal_handlers` is documented as idempotent and installed at most once per process, whatever number of platform handles exist.

**It works without a window.** Installing the handlers does not require the windowing backend to be initialized, so a headless tool (a test harness, a future asset cooker) can use the module for shutdown handling alone.

## What it does not hold

No ECS, no rendering, and no audio device. Audio talks to the OS too, and the boundary here is the concern rather than the fact of being OS-specific.

No file I/O beyond resolving standard paths, since reading files is `liara-assets`. No logical input mapping.

## Replaceability

Replaceable, and meant to be. A platform module built on GLFW, on winit, or directly on Wayland and Win32 is a legitimate substitute, and confining the SDL3 dependency here is precisely what keeps that true.
