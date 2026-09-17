---
title: Where things live
description: A topic-to-repository index, for when you know what you are looking for but not which repository owns it.
sidebar:
    label: Where things live
    order: 11
---

When a topic is added to the project, this table gains a row.

| Topic                                                | Repository                                                        |
|------------------------------------------------------|-------------------------------------------------------------------|
| ECS implementation                                   | `liara-core`                                                      |
| Math implementation                                  | `liara-core`                                                      |
| Logger                                               | `liara-core`                                                      |
| Settings, and the TOML they serialise to             | `liara-core`                                                      |
| Event bus                                            | `liara-core`                                                      |
| Render packet construction                           | `liara-core`                                                      |
| Window creation                                      | `liara-platform`                                                  |
| Input devices                                        | `liara-platform`                                                  |
| OS signals and shutdown requests                     | `liara-platform`                                                  |
| Asset loading and decoding                           | `liara-assets`                                                    |
| Asset handles and lifetime                           | `liara-assets`                                                    |
| Audio playback and mixing                            | `liara-audio`                                                     |
| Vulkan device                                        | `liara-renderer`                                                  |
| GPU upload                                           | `liara-renderer`                                                  |
| ImGui setup                                          | `liara-renderer`                                                  |
| Debug primitive drawing                              | `liara-renderer`                                                  |
| Render packet consumption                            | `liara-renderer`                                                  |
| Every handle and plain-data type in the API          | `liara-interfaces`                                                |
| Render packet structure                              | `liara-interfaces`                                                |
| The interface design rules                           | `liara-interfaces` (`INTERFACES.md`)                              |
| The ABI layout freeze and its oracle                 | `liara-interfaces` (`tests/abi/`, `tools/`)                       |
| Logical input mapping                                | `liara` (launcher)                                                |
| The standalone loop and module composition           | `liara` (launcher)                                                |
| Architecture decision records                        | `liara` (`docs/adr/`)                                             |
| JSON schemas, served over GitHub Pages               | `liara` (`schemas/`)                                              |
| The workspace bootstrap and the preset template      | `liara` (`scripts/`)                                              |
| The AUR `PKGBUILD`, from v0.6                        | `liara` (`packaging/`)                                            |
| Tutorials and user guides, from v0.1                 | Undecided, most likely `liara`. Published as its own site section |
| The Astro preset, design tokens, switchers           | `docs-shared` (`astro/`)                                          |
| The Doxygen XML to Starlight loader                  | `docs-shared` (`astro/api/`)                                      |
| The documentation builder image                      | `docs-shared`                                                     |
| The module registry the switchers read               | `docs-shared` (`modules-registry.json`)                           |
| The documentation hub landing page                   | `docs-shared` (`hub/`)                                            |
| Site tools: fingerprinting, search index, retirement | `docs-shared` (`tools/`)                                          |
| Shared CI workflows, composite actions and scripts   | `.github`                                                         |
| Issue and pull request templates                     | `.github`                                                         |
| Published documentation and the edge worker          | `liara-docs`                                                      |
