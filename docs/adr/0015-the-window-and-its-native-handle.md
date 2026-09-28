---
title: "ADR 0015: The window and its native handle"
description: Why a window is a handle of its own, why the native handle is a tagged union in a shared header, and why events are fixed-size records polled one at a time.
sidebar:
    label: "0015 · The window and its native handle"
    order: 15
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-09-27 | Antoine  |

## Context

The renderer's whole initialisation hangs off one piece of data it cannot produce: the operating system's handle on the window it presents to. Vulkan creates a surface from a different pair per window system, read from the headers rather than recalled: `HINSTANCE` and `HWND` for `vkCreateWin32SurfaceKHR`, a `Display*` and an XID for `vkCreateXlibSurfaceKHR`, a `wl_display*` and a `wl_surface*` for `vkCreateWaylandSurfaceKHR`, an `xcb_connection_t*` and a 32-bit window id for `vkCreateXcbSurfaceKHR`. SDL3, the backend of [liara-platform](../../modules/platform/), exposes the first three as window properties and has no XCB path at all. The handle crosses from the platform module to the host and from the host to the renderer as plain data, so a single `void*` cannot carry it: the renderer has to know which pair it received.

Two neighbouring questions had to be settled in the same stage, because the handle's lifetime and the events that report a window's life depend on them: how a window exists in the contract, and how its events reach the host.

## Decision

Three parts. The behaviour each one produces is described at [liara-platform](../../modules/platform/), and the clause-by-clause contract is the Doxygen of `liara/native_window.h`, `liara/platform/window.h` and `liara/platform/event.h` in `liara-interfaces`.

**A window is a handle of its own**, `liara_platform_window_handle_t`, created from a platform instance and destroyed before it. The instance's create-info did not change, so the stage was additive, an instance with no window never starts SDL's video subsystem, and the native handle's pointers live exactly as long as the window.

**The native handle is a tagged union in a shared header.** `liara_native_window_t` holds a `uint32_t` kind (`NONE`, `WIN32`, `XLIB`, `WAYLAND`) and a union whose members carry the fields of the matching Vulkan create-info under Vulkan's own names, plus a `uint64_t padding[4]` member that fixes the struct at 40 bytes on 32- and 64-bit targets. A new kind is a constant appended to the enum and a member that fits inside the padding, so the struct's size and every existing offset stay put. It lives in `liara/native_window.h`, with the bare `liara_` prefix `naming.md` reserves for what belongs to no single module, because a native window can come from somewhere other than `liara-platform`: an editor embedding the engine in a toolkit widget hands the renderer a window it made itself.

**Events are fixed-size records polled one at a time.** `liara_platform_poll_event` fills one caller-allocated `liara_platform_event_t` (64 bytes: a type, a timestamp, the window concerned, a payload union with padding) per call. The first call of a drain pumps the operating system and the others read what it produced, which bounds the host's loop.

## Alternatives considered

Five, across the three parts:

**The window options in the platform instance's create-info**, one window per instance. Smaller, and the per-instance wording of `liara_platform_quit_requested` fitted it directly. Rejected because the headless case would have become a flag, a second window would have become a breaking change, and the native handle's lifetime would have been the instance's rather than the window's.

**Generic flat fields** (`void* display; void* window; uint64_t window_id;`) instead of a union. Simpler across a foreign-function interface, and it leaves the struct free of unions. Rejected because the meaning of each field changes with the kind: `HINSTANCE` would travel in a field named `display`, and a renderer reading the wrong field of the right struct compiles silently.

**One getter per kind**, `liara_platform_window_get_wayland` and its siblings, the way Vulkan has one create-info per window system. Rejected because it multiplies entry points for no gain: the renderer's create-info still has to carry one tagged value from the host, so the tag has to exist anyway.

**A platform-namespaced type**, `liara_platform_native_window_t` in `liara/platform/`, following the precedent of `liara/renderer/packet.h`, which lives with its consumer. Rejected because the producer is not always the platform module, and a renderer taking a `liara_platform_*` type would name a module it must not know.

**Polling events in batches** into a caller buffer with a capacity and an `out_count`, which is closer to the interface guide's "an array is a pointer and a count". Rejected for v0.1 because it needs an overflow rule and saves a few indirect calls per frame on a handful of events; a batch variant stays additive if input traffic ever measures otherwise.

## Consequences

A window with nothing presented to it is invisible under Wayland and XWayland, since a surface is shown only once a buffer has been committed to it; this was observed under Hyprland before the design was written. Until the renderer presents, `liara-platform` paints a solid placeholder through SDL's window surface, which SDL3's Wayland driver backs with an OpenGL context of its own, having no software framebuffer. That placeholder has to go once a swapchain presents to the same surface, since two producers attaching buffers to one surface would fight.

Adding a kind or an event type is a minor change only because every consumer skips a value it does not know, which the headers state. A consumer that asserts on an unknown kind breaks the promise from its side.

Appending rows to `LIARA_PLATFORM_FUNCTIONS` is additive by the change-to-bump table, and still breaks the workspace superbuild between the interface merge and the module merge, since the launcher takes the address of every listed function. [Continuous integration](../../tooling/ci/#two-tiers) builds same-named branches of sibling repositories together for that reason.

## Revisit if

- A backend's native handle does not fit in the 32 bytes of the union's padding.
- SDL3 exposes an XCB connection, or another platform module brings XCB, which is the moment to add `LIARA_NATIVE_WINDOW_KIND_XCB`.
- The renderer finds a platform that requires `SDL_WINDOW_VULKAN` at window creation, which would add a field to the window create-info.
- The input work of v0.5 measures the per-event call as a cost worth a batch form.
