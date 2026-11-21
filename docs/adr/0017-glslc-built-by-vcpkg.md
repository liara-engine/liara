---
title: "ADR 0017: glslc built by vcpkg"
description: Why the shader compiler comes from vcpkg's shaderc port rather than from the system or the Vulkan SDK, and why a CI with no glslc and a compiler that differed on every machine made that the deciding argument.
sidebar:
    label: "0017 · glslc built by vcpkg"
    order: 17
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-11-21 | Antoine  |

## Context

Stage 5 of [v0.1](../../roadmap/v0-1/) gives `liara-renderer` its first shaders. [liara-renderer](../../modules/renderer/) had settled how they are built before any existed: the GLSL is compiled to SPIR-V at build time by `glslc`, driven from CMake, and the renderer consumes SPIR-V and never compiles GLSL. Where `glslc` comes from was settled too, in [Dependencies](../../tooling/dependencies/): from the system or the Vulkan SDK, installed by the developer, which is the `shaderc` package on Arch and the SDK on Windows, and which `verify` checked for.

That stopped holding the moment a shader had to compile in CI. No runner had `glslc`. The Linux runners install the validation layers and nothing else of Vulkan's, and the Windows runners have no SDK at all. Every CI leg builds the renderer, so every leg would have failed at its first shader.

The compiler also differed on every machine: whatever Arch's `shaderc` is this month on the development machine, whatever Ubuntu 24.04 packages on the Linux runners, whatever SDK a Windows machine installed. A shader compiler is a compiler, and its version decides what a shader compiles to and which warnings `-Werror` turns into failures.

## Decision

**`glslc` comes from vcpkg's `shaderc` port, declared in `liara-renderer/vcpkg.json` as a host dependency** (`{ "name": "shaderc", "host": true }`). vcpkg builds it for the host triplet, at the version the project's baseline pins, like every other dependency.

**CMake uses that `glslc` and no other.** The renderer looks for it in vcpkg's host tools only, with `NO_DEFAULT_PATH`, and fails the configure with a message naming the port when it is missing. A `glslc` on the `PATH` is never picked up silently in its place, since a version nobody chose would defeat the point of pinning one.

The shaders are compiled with `--target-env=vulkan1.3 -Werror -mfmt=c`, plus `-g` in Debug and `-O` otherwise, and embedded in the library. `verify` stops checking for `glslc`. On Windows it keeps requiring `VULKAN_SDK`, which still provides the validation layer.

## Alternatives considered

Two:

**The system's or the SDK's `glslc`, installed on the runners**, which keeps the policy as it was. On Linux that is one `apt install glslc` in the reusable workflows, which is cheap. On Windows it means installing the Vulkan SDK on every run or caching it, and it is a change to `liara-engine/.github` and a release there before any renderer change can pass. Rejected mainly because it keeps three compilers on three kinds of machine, and a shader that compiles cleanly locally and fails under `-Werror` in CI, or the reverse, is the failure pinning prevents.

**SPIR-V committed beside the GLSL**, compiled by whoever changes a shader. No compiler would be needed anywhere but on that person's machine. Rejected because it contradicts compilation at build time, and because a committed binary drifts from its source the first time someone forgets to regenerate it, with nothing to notice.

## Consequences

One shader compiler, pinned by the vcpkg baseline and identical on every machine and every CI leg. The Windows legs compile the shaders without an SDK.

The first configure after the dependency arrives builds `shaderc`, `glslang` and `SPIRV-Tools` for the host triplet, which takes several minutes, then comes from vcpkg's binary cache like everything else. On Linux the host triplet is `x64-linux`, not the `liara-x64-linux` overlay the target dependencies use, so it is built and cached separately.

`glslc` is a build tool only. Nothing links `libshaderc`, and the renderer's and the launcher's `NEEDED` entries say so.

`verify` and the [bootstrap](../../bootstrap/) pages lose a prerequisite.

## Revisit if

- Shaders have to be compiled at run time, for hot-reload or for shaders an asset supplies. The library would then be linked into the renderer anyway, and the question becomes which of `shaderc` and `glslang` to link.
- Building the port becomes a real cost, a cold CI cache on every run for instance.
- The Vulkan SDK becomes a CI requirement for another reason, at which point its `glslc` would come for free and only the pinning argument would remain.
