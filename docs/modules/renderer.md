---
title: "liara-renderer"
description: The reference Vulkan implementation, from device creation to the debug pass, and the one module whose replaceability matters most.
sidebar:
  label: liara-renderer
  order: 8
---

The reference implementation of the renderer interface. It is the most complex module in the project and the one whose replaceability matters most, since it is the one somebody would plausibly rewrite against another graphics API.

It takes render packets and produces pixels. It knows nothing about the ECS, about gameplay or about the editor, and only ever sees the data the host hands it each frame.

## Contents

**The Vulkan device layer.** Instance creation, physical device selection, logical device creation, queue management, command pool management. Built on Vulkan-Hpp and VMA, for the reasons in [ADR 0010](../../adr/0010-vulkan-without-abstraction-layer/), and the one part of this list that exists today: `liara_renderer_create` builds all of it, and `liara_renderer_destroy` tears it down in reverse.

The renderer does not link against the Vulkan loader. It opens `libvulkan` on Linux and `vulkan-1.dll` on Windows when a renderer is created, through Vulkan-Hpp's `vk::raii::Context`, and `VK_NO_PROTOTYPES` makes any direct call to a Vulkan C function a compile error inside the module. [ADR 0016](../../adr/0016-vulkan-loaded-at-run-time/) says why: a binary that contains the renderer starts on a machine with no Vulkan at all, such as a CI runner, and creation then fails with `LIARA_RESULT_NOT_SUPPORTED` and a message.

Vulkan 1.3 is the minimum, for the loader as for the device. Selection rejects any physical device below 1.3 or without a queue family that supports graphics, and, for a renderer given a window, any device without `VK_KHR_swapchain` or without a family that both draws and presents to that window. It then prefers a discrete GPU, then an integrated one, then a virtual one, then a CPU implementation, with ties going to enumeration order. A host may name a preferred GPU in the renderer's create-info (`preferred_device_name`, whose matching rule the header states): the devices that match come first, and when none of them can be used the renderer chooses as without a preference and logs a warning saying why. The logical device has one queue, enables dynamic rendering and synchronization2, which every frame uses, and enables `VK_KHR_swapchain` when it presents.

A debug build requires the Khronos validation layer and refuses to create a renderer without it, naming the package to install. A clean validation log has to mean the layer ran, and a build that silently skipped the layer would report clean every time. A debug build also turns on synchronization validation, since the frames record barriers that the default checks do not inspect. Release builds do not load the layer.

The renderer's messages (the loader's version, the devices it rejected and why, the one it selected, every validation warning and error) are written to stderr as `[liara-renderer] <level>: <message>` lines. That sink is temporary. It stands in for the diagnostics callback [the error-handling rules](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/errors/) describe, until the core's logger exists in v0.2 and gives that callback somewhere to deliver, and every message goes through one internal sink, so the replacement touches nothing else. While the terminal demo covers the terminal and both stdout and stderr are terminals, the sink holds its lines and prints them when the demo ends, with a count of warnings and errors under the grid in the meantime. A crash in that window loses the held lines, and redirecting stderr is the way to keep them.

**The swapchain manager.** Surface creation from the native window handle, which the host obtained from `liara-platform` and passed in the renderer's create-info, then swapchain creation, recreation on resize, image acquisition and presentation. What exists today is all of it but the recreation: a surface on the window, a FIFO swapchain, two frames in flight, and each frame an image cleared to the packet's background colour and presented. A renderer created without a window (`LIARA_NATIVE_WINDOW_KIND_NONE`) has none of it and presents nothing. Once the window is resized or minimised, the swapchain is out of date, and `liara_renderer_submit_frame` returns `LIARA_RESULT_SURFACE_OUT_OF_DATE` (or `LIARA_RESULT_SURFACE_LOST` when the window system takes the surface away) on every later call, until recreation exists.

The create-info is versioned, and what a host may ask of the renderer grows by fields appended in minor versions. The candidates identified so far, none of them needed yet:

- Other ways to designate the GPU than its name. An index into the enumeration, as Godot's `--gpu-index` does, is simple but depends on the loader and the drivers installed. Vendor and device identifiers, as bgfx takes them, are precise for a model, but two identical cards are indistinguishable and nobody knows the numbers by heart. The device UUID (`VkPhysicalDeviceIDProperties::deviceUUID`) is exact on one machine, but a host can learn it only through an enumeration entry point that does not exist. A power preference (low power or high performance), as WebGPU and SDL_GPU offer, needs no knowledge of the devices and cannot name one card.
- The present mode, vsync or not. If it has to change at run time, from an options menu for instance, it is a setter rather than a create-info field.
- The application's name and version, which Vulkan passes on to tools such as RenderDoc and to driver profiles.
- HDR and the colour space.
- Host allocation callbacks, for a host that wants to own the renderer's CPU memory.
- The diagnostics callback, which waits for the core's logger in v0.2 and may not be a create-info field at all.

**The render target abstraction.** Swapchain-backed and offscreen-texture-backed targets, behind the same opaque handle, with image transitions, format negotiation and allocation strategy encapsulated here. This is the piece that makes the editor possible without an interface change later: a viewport panel is a target that is not the swapchain.

**The pipeline cache.** Shader module loading, pipeline state description, pipeline creation, and caching keyed by that state. The renderer consumes SPIR-V and does not compile GLSL, which CMake does at build time through `glslc`.

**The render passes.** The frame logic itself: receiving a packet, sorting by material and pipeline, issuing draw calls, handling transparency, presenting.

**The debug rendering subsystem.** Lines, wireframes, AABBs, frustums and other primitives submitted by the core, and post-v1.0 by the editor for its gizmos. It is a separate pass with a pipeline of its own, which is what lets the editor reuse it without the renderer knowing an editor exists.

**The ImGui integration.** ImGui is drawn here because drawing is what it is. In v0.x it serves the developer console and the stats overlay, and the integration is shaped so that the editor later submits its interface through the editor interface and the renderer draws it unchanged.

## What it does not hold

No ECS, no gameplay logic, no input handling.

No window creation. The window belongs to `liara-platform`, and the renderer receives a native handle from the host and nothing else.

No asset loading. The renderer is handed prepared CPU-side data by the host, which got it from `liara-assets`, and it uploads that to the GPU. It never reads a file and never calls the assets module.

## Internal layout

Internal, not part of any contract, and free to change. The shape it is heading for follows the list above: `src/device/`, `src/swapchain/`, `src/targets/`, `src/pipelines/`, `src/passes/`, `src/debug/`, `src/imgui/` and `src/platform/`. Today `src/` holds `device/` (the loader, the instance, device selection, the device and the allocator), `swapchain/` (the surface, the swapchain and the pure functions choosing its format, extent and image count), `frame/` (the frames in flight and the clear), `console/` (the terminal renderer that still draws the packet), the `Renderer` class composing them, the create-info checks, the temporary log sink, and `renderer.cpp`, the ABI shim.

`shaders/` holds the GLSL source of the engine's built-in shaders, compiled to SPIR-V at build time and either embedded in the binary or shipped next to it, controlled by a CMake option.
