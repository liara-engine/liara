---
title: Modularity model
description: Why modules are selected at build time, why their interfaces are designed as if they were not, and the two trades that keep a future reorganisation cheap.
sidebar:
  order: 2
---

The engine is a collection of separately versioned libraries linked together at build time. The build system decides which modules are present, and a shipped game is one executable with its modules linked in, with no plugin discovery mechanism to debug.

A runtime-loading path exists alongside it and is described below. It is no longer experimental, and it is no longer only a test of the boundary: it is the only way to load a module that this build system did not build, which means it is how a module written in another language is loaded at all. A shipped Liara game still links its modules in, because that is the simplest thing that works for a game shipping its own engine.

## Why build-time selection

Three models were on the table.

| Model                                                | Used by  | Cost                                                            |
|------------------------------------------------------|----------|-----------------------------------------------------------------|
| Build-time selection through build system options    | Bevy     | Substituting a module means recompiling                         |
| Runtime loading of dynamic libraries from a manifest | Unreal   | No recompilation, and an ABI versioning problem at run time     |
| One process per component, talking over IPC          | Hyprland | Excellent for desktop tooling, unworkable at frame-loop latency |

The first was chosen because it delivers the modularity this project actually wants, meaning clean interfaces, replaceability in principle and forced discipline, without the operational complexity of plugin loading. Distribution is drastically simpler too: a game ships as one executable plus its assets.

## Designed for dynamic, implemented as static

Modules are linked statically today, and the interfaces between them are designed as though they were not. Every boundary uses C linkage, plain-old-data types, opaque handles and explicit version negotiation.

The discipline costs little at the boundary, and what it buys is that switching to runtime loading later is a matter of writing a loader rather than rewriting contracts. It also buys something independent of any loader: a module can be replaced by an implementation in another language that implements the same C interface. That is checked rather than assumed — a Rust library exporting `liara_platform_*`, including no Liara header and linking against nothing, is loaded and version-negotiated by an unmodified launcher. Such a module is reachable through the `-runtime` presets and through nothing else, since linking one at build time would mean naming a CMake target for it.

The claim is checked rather than asserted. One CI leg builds every module as a shared library and links the launcher against them, and another makes the launcher resolve them at run time instead, through the `-link` and `-runtime` presets described in [CI](../../tooling/ci/#the-build-matrix). A module that quietly acquired a link-time dependency on a sibling, or leaked C++ across its boundary, fails one of the two. The static-versus-dynamic choice is therefore a pair of build flags rather than an architectural fork.

## One namespace per subsystem, from the first line

A subsystem that could plausibly become its own module gets its own ABI namespace immediately, whatever repository implements it today. That costs nothing at the time. Retrofitting it costs a major bump of `liara-interfaces` and a forced migration of every consumer.

It is the same trade as the previous section, one level up. There, the interfaces are designed as if modules were dynamic although they are linked statically. Here, they are designed as if every subsystem had its own repository although several share one. Both keep a future reorganization a matter of moving files rather than rewriting contracts.

## Multi-repository layout

The codebase is split across Git repositories, one per module, under the `liara-engine` organization. It prioritizes separation over operational convenience, and the tradeoff is worth stating plainly.

A monorepo would be easier to refactor across, easier to bootstrap, and easier to keep version-locked. The multi-repository layout needs coordination across pull requests, a workspace bootstrap for local development, and an explicit record of which versions work together.

It is preferred because the cognitive cost of mixing unrelated concerns in one repository is, for me specifically, higher than the cost of running a bootstrap script. [ADR 0001](../../adr/0001-multi-repository-layout/) records what that judgment is based on, which is the previous engine and how it ended. The discipline imposed by separate repositories is the feature.

The repository list itself is in [Modules](../../modules/).
