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

Vulkan 1.3 is the minimum, for the loader as for the device. Selection rejects any physical device below 1.3 or without a queue family that supports graphics, then prefers a discrete GPU, then an integrated one, then a virtual one, then a CPU implementation, with ties going to enumeration order. Nothing lets a host force a choice yet; that belongs to the renderer's create-info, which arrives with the swapchain. The logical device has one graphics queue and enables no extension and no optional feature until something uses one.

A debug build requires the Khronos validation layer and refuses to create a renderer without it, naming the package to install. A clean validation log has to mean the layer ran, and a build that silently skipped the layer would report clean every time. Release builds do not load it.

The renderer's messages (the loader's version, the devices it rejected and why, the one it selected, every validation warning and error) are written to stderr as `[liara-renderer] <level>: <message>` lines. That sink is temporary. It stands in for the diagnostics callback [the error-handling rules](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/errors/) describe, until the core's logger exists in v0.2 and gives that callback somewhere to deliver, and every message goes through one internal sink, so the replacement touches nothing else.

**The swapchain manager.** Surface creation from the native window handle, which the host obtained from `liara-platform` and passed in at initialization, then swapchain creation, recreation on resize, image acquisition and presentation.

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

Internal, not part of any contract, and free to change. The shape it is heading for follows the list above: `src/device/`, `src/swapchain/`, `src/targets/`, `src/pipelines/`, `src/passes/`, `src/debug/`, `src/imgui/` and `src/platform/`. Today `src/` holds `device/` (the loader, the instance, device selection, the device and the allocator), `console/` (the terminal renderer that still draws the packet), the `Renderer` class composing the two, the temporary log sink, and `renderer.cpp`, the ABI shim.

`shaders/` holds the GLSL source of the engine's built-in shaders, compiled to SPIR-V at build time and either embedded in the binary or shipped next to it, controlled by a CMake option.
