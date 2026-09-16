---
title: "ADR 0007: CMake with presets, and vcpkg in manifest mode"
description: Why presets are the only supported way to configure the build, why dependencies come from vcpkg, and what the first configuration on a fresh machine costs.
sidebar:
  label: "0007 · CMake and vcpkg"
  order: 7
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-08-26 | Antoine  |

*Retrospective.*

## Context

Six repositories, two platforms, three compilers, two linkages and two build types have to produce the same result on my machine and in CI, including on a machine where none of it has ever been built before. Two questions follow: what drives the build, and where dependencies come from.

They are less separable than they look. vcpkg's manifest mode is driven by a CMake toolchain file, and the toolchain file is set by a preset, so deciding one narrows the other. Recording them apart would misrepresent how it went.

The Windows requirement shapes both. I develop on Arch and validate on Windows, and "works on my machine" has to mean something on a machine that is not mine.

## Decision

CMake 3.29 or newer, configured only through presets. The presets are the supported interface: there is no documented way to configure the project by passing options on a command line.

Dependencies come from vcpkg in manifest mode. Each repository declares its direct dependencies in `vcpkg.json` and pins the same registry baseline in `vcpkg-configuration.json`. Submodules and `FetchContent` are not used for external dependencies, with a narrow exception for header-only libraries missing from the registry, granted case by case through a record of its own.

:::note[One option does cross the line]
The workspace-setup action passes `-DLIARA_INTERFACES_REQUIRE_CROSS_LANGUAGE_TESTS` on top of the preset, ON for the one leg that owns the Zig and Rust tests and OFF everywhere else. It is the only flag CI adds, it exists because the toolchains are optional locally and mandatory on that leg, and the ABI portability workflow keeps a `configure-options` input for the same kind of case. So the rule is that presets carry every build configuration and CI adds no flag that changes what gets built, not that the command line is never touched.
:::

## Alternatives considered

**Meson or Bazel instead of CMake.** Both are better designed than CMake, and I do not think that is arguable. What settled it is the ecosystem: Vulkan, VMA, SDL, Dear ImGui and effectively every C++ dependency I will want ships CMake support first, vcpkg integrates as a CMake toolchain file, and IDE support for presets is now broad. Picking the better-designed tool would mean writing and maintaining the glue CMake gets for free, and that glue is not what I want to be spending this project on.

**Configuring through command-line options, with presets as a convenience.** This is how most projects work and it is what I moved away from. With options on the command line, the set of supported configurations is unbounded and nothing verifies any of it. With presets as the only interface, every CI leg is reproducible locally by name, which is why the matrix is expressed as preset names instead of (OS, compiler, build type) tuples, and why I do not add a leg I cannot name.

**Conan instead of vcpkg.** A real contender, and better on some axes, since Python recipes are more expressive than portfiles and version ranges are more flexible. I went with vcpkg because MSVC support is first-class by construction (Microsoft maintains it), because manifest mode drops straight into `find_package` with no adapter, and because a triplet states target, linkage and build type in one place where Conan profiles spread the same information around.

**Distribution packages**, `pacman`, `apt`, `dnf`. Zero setup on Linux and always fastest. Rejected because Arch, Ubuntu and Fedora ship different versions of the same library, so the build reproduces the distribution rather than the project, and because there is no Windows equivalent, which would mean maintaining two dependency stories.

**Submodules or `FetchContent`.** Submodules are forgotten, break on detached HEADs and complicate every clone. `FetchContent` re-downloads and rebuilds sources on every fresh build, which is exactly the cost vcpkg's binary cache exists to remove.

## Consequences

The first configuration on a fresh machine is slow, because vcpkg builds every dependency from source. The binary cache makes everything after that fast, but the first impression anyone gets of this project is that wait. It is the largest cost of this decision and it lands at the worst moment.

Presets are generated from one template in the meta repository rather than committed per repository. One copy per repository would drift, so the branch CI tier fetches the template instead. The side effect is that a module repository has no `CMakePresets.json` in Git, which surprises anyone who clones one expecting to build it directly.

Windows uses a multi-config generator and Ninja does not, so a build preset's name does not always match its configure preset's name, and `CMAKE_BUILD_TYPE` means nothing on Windows. That is why Linux ends up with twelve presets (four names across two build types and two compilers, each also in a `-link` and a `-runtime` variant) and Windows with three configure presets and six build presets. Everything that needs a build directory resolves it through the preset file.

The baseline pin has to be identical across repositories, because the workspace merges the manifests and resolves once. Two repositories on different baselines would produce a build where one silently gets versions it never declared, so the bootstrap script compares them and refuses to continue rather than picking one.

CMake is still CMake: a macro language with global state where a mistake in one directory can affect another. `liara_configure_target` concentrates the part most likely to diverge across modules, which reduces the exposure without removing it.

## Revisit if

- vcpkg stops carrying a dependency I need and I have to maintain the port myself, which is where Conan's recipe model would start being worth a migration.
- C++20 modules become worth adopting, since build-system support is where CMake's lead over Meson is thinnest.
- First-build time becomes the real barrier to anyone contributing, which argues for publishing a prebuilt binary cache rather than for changing tools.
