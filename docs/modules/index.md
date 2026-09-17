---
title: Modules
description: The four kinds of repository, the rule that decides whether something is a module, and when each one is created.
sidebar:
  label: Overview
  order: 0
---

Where [Modularity model](../architecture/modularity/) explains why the project is modular, these pages specify what is actually built and where it lives. One page per repository, plus the boundaries between them.

Everything is hosted under the `liara-engine` organization, and everything falls into one of four kinds. The kind matters more than the list, because it decides what a repository is allowed to do.

## The contract

| Repository         | Role                                                  | Status  |
|--------------------|-------------------------------------------------------|---------|
| `liara-interfaces` | The C ABI headers every module implements or consumes | Phase 0 |

The contract is not a module. It contains no implementation and exports no symbol, and everything else in the project either implements a part of it or consumes it.

## The modules

A module implements one namespace of the contract, versions on its own cadence, and is replaceable in principle by an alternative implementation, including one written in another language.

| Repository       | ABI namespace      | Role                                                 | Introduced |
|------------------|--------------------|------------------------------------------------------|------------|
| `liara-core`     | `liara_core_*`     | ECS, math, logger, settings, events, loop primitives | Phase 0    |
| `liara-platform` | `liara_platform_*` | Window, input devices, OS signals, timing            | v0.1       |
| `liara-renderer` | `liara_renderer_*` | Reference Vulkan renderer                            | Phase 0    |
| `liara-assets`   | `liara_assets_*`   | Loading, decoding and lifetime of asset data         | v0.3       |
| `liara-audio`    | `liara_audio_*`    | Audio playback and mixing                            | v0.5       |
| `liara-physics`  | `liara_physics_*`  | Rigid bodies, collision, queries                     | v1.x       |

`liara-core` is the one module that is not replaceable, since it owns the data model everything else agrees on. It is still a module rather than a privileged runtime, because it obeys the same rules (one ABI namespace, one `info()` entry point, its own version) and because a subsystem that cannot be described through the contract does not belong in it.

## The hosts

A host composes modules: it creates them, checks their ABI versions against each other, owns the application loop, and moves data between them. A host is not a module, since it exports nothing and nobody links against it.

| Repository     | Role                                                                      | Introduced |
|----------------|---------------------------------------------------------------------------|------------|
| `liara` (meta) | The launcher, plus everything that has to know about every module at once | Phase 0    |
| `liara-editor` | The editor application                                                    | v1.x       |

## The infrastructure

Consumed by CI and by the documentation pipeline, never by CMake.

| Repository    | Role                                                                | Status  |
|---------------|---------------------------------------------------------------------|---------|
| `.github`     | Organization-level reusable workflows, shared CI scripts, templates | Phase 0 |
| `docs-shared` | The Astro preset, the site tools, the builder image                 | Phase 0 |
| `liara-docs`  | Documentation hosting and the edge worker                           | Phase 0 |

## The rule that defines a module

One module is one ABI namespace, and the three spellings never diverge: the include namespace `liara/<name>/`, the symbol prefix `liara_<name>_*`, and the self-description entry point `liara_<name>_info()`.

A subsystem gets its namespace from its first line of code, before anyone knows whether it will ever move into a repository of its own. Moving an implementation between repositories is cheap. Renaming a symbol consumers already call is a major bump of `liara-interfaces` and breaks every one of them. The repository layout is a delivery decision and stays revisable; the namespace is a contract and does not.

The substitution test follows from that, and it is what [ADR 0004](../adr/0004-module-boundaries/) applies: could this be reimplemented in another language against the same C interface and swapped in? If not, it is not a module and does not get a repository.

## When each one appears

A repository is created when its first line of code is written, and not before. `liara-platform` therefore arrives with v0.1, `liara-assets` with v0.3, `liara-audio` with v0.5, and `liara-physics` and `liara-editor` with v1.x. Until then they exist in these pages and nowhere else, which is deliberate: the namespace is claimed from the start, the repository is not.
