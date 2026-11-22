---
title: "ADR 0019: The renderer recreates its own swapchain"
description: Why the host pushes the window's size to the renderer but the renderer recovers from an out-of-date swapchain on its own, so that a host which forgets to react no longer freezes its window.
sidebar:
    label: "0019 · The renderer recreates its swapchain"
    order: 19
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-11-22 | Antoine  |

## Context

A swapchain has to follow its window. Two facts say when it no longer does, and they come from two different modules. That the window was resized is a fact of [liara-platform](../../modules/platform/), which reports it as `LIARA_PLATFORM_EVENT_WINDOW_RESIZED` in its event queue. That the swapchain is out of date, or that the surface is gone, is a fact of [liara-renderer](../../modules/renderer/), which Vulkan reports from inside a frame, when an image is acquired or presented. While planning ABI 1.0.0 on 2026-09-20 the two were kept apart, and neither became a callback: the host pushes the resize to the renderer, and the renderer returns the out-of-date state as a `liara_result_t` from `liara_renderer_submit_frame`, to which "the host reacts synchronously in its own loop".

Stage 4 of [v0.1](../../roadmap/v0-1/) built the second half and left the first to stage 6: an out-of-date swapchain stayed out of date, and the window froze at its first size. The window systems do not report alike either. X11 and Win32 give a surface a size of its own and report an out-of-date swapchain once the window changes. Wayland gives a surface no size at all and reports nothing on a resize, so there the renderer can only learn the window's size from the host.

What was left open was who recreates the swapchain once Vulkan has said it is out of date.

## Decision

**The host pushes the window's size through `liara_renderer_window_resized`.** It passes on the last `LIARA_PLATFORM_EVENT_WINDOW_RESIZED` of each drain of its events. The renderer keeps the size and adapts its swapchain at the next `liara_renderer_submit_frame`, so a drag that reports thirty sizes between two frames costs one recreation. Under Wayland this push is the only signal there is.

**The renderer recreates its swapchain on its own**, before the frame, whenever something asks for it: a size pushed that differs from the swapchain's, an earlier acquisition or presentation that found it out of date or suboptimal, or a window that had no area and has one again. Where the surface has a size of its own, that size wins over the pushed one.

**`LIARA_RESULT_SURFACE_OUT_OF_DATE` describes one call.** It says that nothing was presented, because the swapchain no longer matched the window or because the window has no area, and it no longer sticks. The host's only obligation towards it is to know that the frame presented nothing. `LIARA_RESULT_SURFACE_LOST` still sticks, and what to do about it stays the host's decision.

## Alternatives considered

Two:

**Recreation driven by the host.** The renderer would recreate only when pushed a size; an out-of-date result would stick until then, and the host would react to it by reading `liara_platform_window_get_pixel_size` and pushing it. It is the most literal reading of the September decision, and the renderer would never guess a size. It lost because every host has to write that reaction, a host in another language included, and a host that forgets it gets exactly the frozen window of stage 4, with nothing to say why.

**The window's size passed with every frame**, as an argument of `liara_renderer_submit_frame`. The renderer could never miss a resize, and there would be no new entry point. It lost because it makes the host track and pass the size on every frame for nothing the push does not already give, and it ties the frame's call to a window that a renderer drawing offscreen does not have.

## Consequences

A host that does nothing with `SURFACE_OUT_OF_DATE` still gets a window that follows its size on X11 and Win32. Under Wayland, a host that never pushes the size gets images of the size the window had at creation, which is the one thing the push cannot be done without.

The launcher handles every result of `liara_renderer_submit_frame` in one place: an out-of-date frame makes it sleep until the next tick, since nothing blocked on presentation, and a lost surface, a refused packet or any other failure ends it with an error and exit code 1.

A recreation waits for the GPU to be idle. That stall is invisible at the rate windows are resized, and it is what lets the old images, their semaphores and their fences go without tracking which frame still uses them.

## Revisit if

- A target platform reports neither a size of its own nor an out-of-date swapchain, so that neither signal reaches the renderer.
- A renderer presents to several windows, and the size it is pushed has to name which one.
- A host needs to defer a resize that the renderer would otherwise notice on its own, during a drag for instance.
