---
title: Build system
description: Why presets are the only supported way to configure Liara, how the preset matrix is laid out, and what changes between Linux and Windows.
sidebar:
  order: 1
---

## CMake, driven by presets

CMake 3.29 or newer, configured through presets and nothing else. There is no documented way to configure the project by passing options on a command line, and each non-hidden preset is invokable by name from a terminal and used under that same name by CI.

The presets live in the meta repository as a template, from which the bootstrap script generates the workspace's `CMakePresets.json`. A module repository carries none of its own, because a module is configured as part of the workspace and a second set per repository would be copies of the same file drifting apart. The visible cost is that cloning `liara-core` alone gives you no way to configure it, which can surprises people.

:::note[CI adds exactly one flag]
The workspace-setup action passes `-DLIARA_INTERFACES_REQUIRE_CROSS_LANGUAGE_TESTS` on top of the preset, ON for the leg that owns the Zig and Rust tests and OFF everywhere else, and the ABI portability workflow keeps a `configure-options` input for the same kind of case. The rule is that presets carry every build configuration and CI adds no flag that changes what gets built, which is narrower than "the command line is never touched".
:::

## The preset matrix

Presets are generated from one template and organized along three axes: platform and compiler, build type, and linkage.

| Suffix     | `BUILD_SHARED_LIBS` | `LIARA_MODULE_LOADING` |
|------------|---------------------|---------------------------------|
| *(none)*   | `OFF`               | `link`                          |
| `-link`    | `ON`                | `link`                          |
| `-runtime` | `ON`                | `runtime`                       |

`LIARA_MODULE_LOADING` is an explicit cache variable rather than something inferred from `BUILD_SHARED_LIBS`, because "built as a shared library" and "loaded at runtime" are two different questions. The middle combination is the one that catches export-macro mistakes, and inferring one from the other would remove it.

**`link` is the C and C++ path, and only that.** Linking a module at build time means naming a CMake target for it, so the two `link` rows require every module to be built from this workspace by this build system. `runtime` requires only a shared library exporting the right symbols, which is what a module written in any language with a C FFI can produce. That makes the difference between the rows a question of what a module *is*, not only of how fast it starts: a Rust or Zig module is reachable through `-runtime` and through nothing else. [ADR 0012](../../adr/0012-generated-module-dispatch-tables/) covers how a host calls one either way without the call site knowing which.

On Linux that gives twelve presets: `linux-<debug|release>-<gcc|clang>`, each also in a `-link` and a `-runtime` variant. `linux-debug-clang` is what `liara.sh` uses when you do not name one.

## Single-config on Linux, multi-config on Windows

Ninja fixes the build type at configure time, so on Linux each build type is its own configure preset and its own build tree. The Visual Studio generator chooses at build time instead, which means `CMAKE_BUILD_TYPE` has no effect there. The Windows configure presets do not set it, and the build and test presets carry a `configuration` field, which is the declarative form of `--config`.

Windows therefore has three configure presets (`windows`, `windows-link`, `windows-runtime`) and six build presets pairing each with a configuration. One configure preset serving both Debug and Release is also why switching between them on Windows does not re-resolve dependencies.

The consequence for anything automated: a build preset's name does not always match its configure preset's name. Resolve the build directory through the preset file rather than guessing it from a name.

## Out-of-source builds

Build artifacts land in `build/<preset-name>/` and the source tree contains none of them. CMake refuses an in-source build with an explicit message.

Inside a build tree, artifacts are grouped by kind rather than by the module that produced them. Executables and shared libraries go to `build/<preset>/bin/`, static and import libraries to `build/<preset>/lib/`, with multi-config generators adding a configuration subdirectory to each. The launcher and the modules it may load at runtime are therefore siblings, and the launcher's rpath is `$ORIGIN` rather than an absolute path into the build tree. That is the arrangement a packaged install needs, established now so that packaging later does not have to relocate anything.

## The compile commands database

Every Linux build generates `compile_commands.json`, and the bootstrap script symlinks it to the workspace root so that clangd finds it without configuration. VSCode, Neovim and CLion all consume it through clangd.

The same file is what clang-tidy reads, which is why `liara.sh check` builds before it runs tidy. It is never committed, and it has to be there for local tooling to work at all.

The Visual Studio generator does not produce one, so the symlink step is skipped on Windows and clangd-based tooling there needs a Ninja configuration of its own.

## The compile cache and the linker

The generated workspace `CMakeLists.txt` picks a compiler cache (sccache if present, otherwise ccache) and a linker (mold, otherwise lld) before it creates any target, which is the only point at which those settings take effect. The presets configure neither. Both are found on `PATH`, both are optional, and their absence is silent.

CI deliberately installs neither, for reasons that belong with the [CI matrix](../ci/#parallelism-and-caching).
