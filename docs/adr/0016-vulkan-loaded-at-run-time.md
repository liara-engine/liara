---
title: "ADR 0016: Vulkan loaded at run time"
description: Why the renderer opens the Vulkan loader when it is created instead of linking against it, and why a CI with no Vulkan made that the deciding argument.
sidebar:
    label: "0016 · Vulkan loaded at run time"
    order: 16
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-11-10 | Antoine  |

## Context

Stage 3 of [v0.1](../../roadmap/v0-1/) gives `liara-renderer` its first Vulkan code: `liara_renderer_create` creates an instance, selects a physical device, and creates a logical device, a queue, a command pool and a VMA allocator. The usual way a program reaches Vulkan is to link against the loader, `libvulkan.so.1` on Linux and `vulkan-1.dll` through its import library on Windows, which CMake exposes as `Vulkan::Vulkan`. The loader then becomes a load-time dependency of whatever links the renderer: the launcher itself under the static presets, and `liara_renderer.so` under `-link` and `-runtime`.

CI has no Vulkan. I deferred that question on 2026-09-20 because it was too early to weigh its cost against what it would catch, and until it is weighed, "no Vulkan validation errors in debug builds" is a locally verified item of the v0.1 Definition of Done. The standard GitHub-hosted runners the workflows use have no GPU, nothing in the workflows installs a loader, and whether a runner image happens to ship one is not something the project relies on.

`launcher_smoke` runs in CI under every preset, headless: it loads every module, negotiates, and creates the instances that need no window. From stage 3 it skips the renderer instance, as it already skipped the window, so that it never asks for a GPU. That skip only helps if the launcher starts. With the loader linked, the static launcher fails in the dynamic linker before `main` on a machine without it, and under `-runtime` the launcher's `dlopen` of `liara_renderer.so` fails on the unsatisfied `NEEDED` entry. The test that exists to prove the modules load would fail for a reason that has nothing to do with the modules.

## Decision

**`liara-renderer` has no link-time dependency on the Vulkan loader.** The first object `liara_renderer_create` builds is a `vk::raii::Context`, whose default constructor opens the loader through Vulkan-Hpp's `DynamicLoader` (`libvulkan.so`, then `libvulkan.so.1`, on Linux; `vulkan-1.dll` on Windows) and resolves `vkGetInstanceProcAddr`. Every later call goes through the dispatch tables `vk::raii` builds from that pointer and from `vkGetDeviceProcAddr`. VMA receives the same two pointers through `VmaVulkanFunctions`, with `VMA_STATIC_VULKAN_FUNCTIONS` set to 0 and `VMA_DYNAMIC_VULKAN_FUNCTIONS` to 1.

**`VK_NO_PROTOTYPES` is defined on the `liara_renderer` target.** The Vulkan C prototypes are then not declared at all inside the module, so a direct call to `vkCreateInstance` is a compile error rather than an undefined symbol that a link step, or a machine without the loader, discovers later. Two checks hold the decision in place: `nm -D --undefined-only` on a shared `liara_renderer.so` lists no symbol starting with `vk`, and `readelf -d` on a statically linked `liara_launcher` lists no `libvulkan` among its `NEEDED` entries.

A machine without a loader gets `LIARA_RESULT_NOT_SUPPORTED` from `liara_renderer_create`, after the renderer has said why. [liara-renderer](../../modules/renderer/) describes that behaviour, and the Doxygen of `liara_renderer_create` in `liara-interfaces` lists the code.

## Alternatives considered

Three:

**Linking against the loader**, through `Vulkan::Vulkan`. It is what nearly every Vulkan program does, it needs no configuration, and it was the default until the CI made the question concrete. Rejected because it makes the loader a requirement for starting any binary that contains the renderer, and in CI that is every launcher under every preset. Calling device-level functions through pointers from `vkGetDeviceProcAddr` also skips the loader's dispatch trampoline, which is a side effect of the decision and not a reason for it: nothing in v0.1 is measured closely enough for that cost to show.

**volk**, the meta-loader that opens the library at run time and fills global function pointers. It does the job well and is widely used, and under the rule [ADR 0010](../0010-vulkan-without-abstraction-layer/) states for letting a library in, it would qualify. Rejected for redundancy: Vulkan-Hpp's dispatcher already opens the loader and resolves the functions, and volk would add a second dependency and a second way to call Vulkan inside one module, global pointers beside per-object dispatchers.

**A software Vulkan implementation in CI**, lavapipe from Mesa on the Linux runners, so that linking works there and the `gpu` tests could even run. Rejected because it would settle the question I deferred as a side effect of a linking choice, and because the Windows runners would need an answer of their own. That question deserves its own weighing, and this record lists it below as the first reason to reopen.

## Consequences

A machine without Vulkan starts the launcher, and the renderer refuses with a message instead of the dynamic linker refusing before `main` with one of its own. That is true on a developer's machine as much as in CI, and it is the better failure of the two, since the renderer's message can name what to install.

No Vulkan C function can be called directly inside `liara-renderer`. A snippet taken from a tutorial, or a helper written against the C prototypes, has to go through the dispatcher or does not compile. A third-party library brought in later that calls Vulkan itself has to accept function pointers the way VMA does. The next one due is ImGui, whose Vulkan backend is known to offer a function-loading entry point for this case (`ImGui_ImplVulkan_LoadFunctions`, behind `IMGUI_IMPL_VULKAN_NO_PROTOTYPES`); that has not been checked against the version the project will pin.

`vulkan.hpp` and `vulkan_raii.hpp` come from vcpkg's `vulkan-headers` port, while the loader that runs is whichever one the system's library search finds. Their versions are independent: the pinned baseline ships headers 1.4.341, and the development machine runs loader 1.4.363. Linking would not have tied them together either, since the loader is a shared library in both arrangements.

## Revisit if

- Vulkan arrives in CI, which is the deferred decision of 2026-09-20 being taken. Linking becomes possible again, and the question left is whether the compile-time guarantee of `VK_NO_PROTOTYPES` is worth keeping on its own.
- A library the renderer needs calls Vulkan through the C prototypes and cannot be handed function pointers.
- Vulkan-Hpp removes `DynamicLoader`, or `vk::raii::Context` stops opening the loader itself.
