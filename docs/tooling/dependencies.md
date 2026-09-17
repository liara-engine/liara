---
title: Dependencies
description: How vcpkg manifest mode works here, why the registry baseline has to match across repositories, and how Liara's own modules find each other.
sidebar:
  order: 2
---

## vcpkg in manifest mode

Each repository declares its direct dependencies in `vcpkg.json` and pins a registry baseline in `vcpkg-configuration.json`. When CMake configures, it invokes vcpkg, which reads the manifest and makes the declared dependencies available. The first configuration on a fresh machine downloads and builds them, and every configuration after that uses cached binaries.

The baseline has to be identical across repositories, and that is a constraint rather than an observation. The workspace merges every manifest into one and resolves once, so two modules pinned to different baselines would produce a build in which one of them silently gets versions it never asked for. The bootstrap script compares them and refuses to proceed when they disagree, instead of picking one.

The reasoning behind vcpkg over Conan or distribution packages is in [ADR 0007](../../adr/0007-cmake-presets-and-vcpkg/). The short version is that Microsoft maintaining it makes MSVC first-class, manifest mode drops straight into `find_package` with no adapter, and a triplet states target, linkage and build type in one place.

## The binary cache

CI keeps a per-leg GitHub Actions cache over `VCPKG_DEFAULT_BINARY_CACHE`, keyed by the runner OS, the preset, and a hash of the merged manifest, so it invalidates itself when any of those change. Locally, vcpkg uses its own per-developer cache directory with no setup.

## Submodules and FetchContent

Neither is used for external dependencies. Submodules get forgotten, break on detached HEADs and complicate every clone. `FetchContent` re-downloads and rebuilds sources on every fresh build, which is exactly the cost the binary cache exists to remove.

The exception is a header-only library missing from the vcpkg registry, where re-downloading a few kilobytes costs nothing. It is granted case by case, through a record of its own.

## What each module declares

A dependency belongs to the module that owns the concern, which is what keeps these lists short. The moment a module needs a library the boundary says belongs elsewhere, the boundary is what is wrong.

This is the target, built up progressively across v0.x:

| Module             | Dependencies                                                                                                                                             |
|--------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `liara-interfaces` | None. The contract has to be includable by anything, including a consumer that has never heard of vcpkg                                                  |
| `liara-core`       | doctest (tests only), tomlplusplus for settings, glm as an internal implementation detail never exposed across the ABI                                   |
| `liara-platform`   | doctest (tests only), SDL3                                                                                                                               |
| `liara-renderer`   | doctest (tests only), Vulkan-Headers, vulkan-memory-allocator, imgui, and either glslang or shaderc if runtime shader compilation turns out to be needed |
| `liara-assets`     | doctest (tests only), stb, and either cgltf or tinygltf                                                                                                  |
| `liara-audio`      | doctest (tests only), miniaudio                                                                                                                          |

Today the lists are shorter than that table: `liara-core/vcpkg.json` declares no runtime dependency at all, with doctest behind its `tests` feature, because settings and math have not been written yet.

Logging is deliberately missing from the table. Whether `liara-core` uses spdlog or its own logger is undecided, and the decision belongs to v0.2 rather than to a line in a manifest.

The Vulkan SDK is not managed by vcpkg. It is a system dependency the developer installs, with instructions in [Bootstrap](../../bootstrap/).

## Between Liara's own modules

vcpkg has nothing to do with these. They resolve differently depending on which of the two builds you are in.

In a **workspace build**, the generated superbuild `CMakeLists.txt` pulls each module in with `add_subdirectory()`, so inter-module dependencies are targets in one build tree.

In a **consumer build**, each module installs CMake config files through `install(EXPORT ...)` and consumers reach them with `find_package(LiaraInterfaces REQUIRED)` and friends. Modules are always consumed through their `Liara::` alias rather than the bare target name, so that a test compiles against exactly what an external consumer would get.

Both flows run in CI on every push, which is the only thing that keeps the second one working, since nothing in day-to-day development exercises it.

## Renovate

Every repository carries a `renovate.json`, and Renovate is installed at the organization level. It opens the pull requests that move the baseline and the action versions, which then go through the same CI as anything else.
