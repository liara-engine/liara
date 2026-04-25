# Liara Engine

> A modern 3D game engine, built from scratch in C++ with Vulkan, as a
> personal learning project. Modular by construction, cross-platform,
> and honest about what it is.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Vulkan 1.3](https://img.shields.io/badge/Vulkan-1.3-red.svg)](https://www.vulkan.org/)
[![Linux](https://img.shields.io/badge/Linux-supported-success.svg)](#)
[![Windows](https://img.shields.io/badge/Windows-supported-success.svg)](#)

---

## Status

**Current version:** Phase 0 (bootstrap) — see
[`docs/ROADMAP.md`](./docs/ROADMAP.md).

This is a **reboot** of an earlier engine project. The earlier
project taught lessons; this one applies them. The goal is not to
build a Unity competitor; it is to build a clean, modular engine
that the author understands top to bottom and that, by version 1.0,
is good enough to ship a small game with.

The project is openly developed, MIT-licensed, and welcomes
spectators, suggestions, and contributors. It is also openly a
**personal project** with one primary developer; pace and priorities
reflect that.

---

## What Is This?

Liara is a 3D game engine composed of independently developed
modules:

- **`liara-interfaces`** — C ABI headers that define the contract
  between modules. Header-only.
- **`liara-core`** — engine foundation: ECS, math, asset management,
  logger, settings, application loop.
- **`liara-renderer`** — Vulkan reference renderer, the engine's
  "graphics backend".
- **`liara-editor`** *(post-v1.0)* — visual editor, Unity-style.
- **`liara-physics`** *(post-v1.0)* — physics module.

The modules communicate through a stable C interface, which means
any of them can in principle be replaced by an alternative
implementation, in any language. The C++ implementation is the
reference, not the only possibility.

Each module lives in its own repository under the
[`liara-engine`](https://github.com/liara-engine) GitHub
organization. This meta repository orchestrates the lot.

---

## Why Another Engine?

Three honest reasons:

1. **Learning.** Modern graphics programming, Vulkan, modern C++,
   large-scale software architecture, build systems, CI/CD. Building
   a game engine touches all of these and forces good answers.
2. **Curiosity.** Wanting to understand how engines work, in detail,
   not just at the API level.
3. **Joy.** Some people garden; some people build engines.
   Productivity is not the metric here.

Liara does not aim to be the next Unity. It aims to be an engine
that, by v1.0, lets a sufficiently motivated developer build and
ship a small 3D game without feeling cheated.

---

## Quick Start

Build the engine and run the demo (assuming you have Arch Linux
with Vulkan SDK and CMake 3.29+):

```bash
git clone https://github.com/liara-engine/liara.git
cd liara
./scripts/setup-workspace.sh
cd workspace
cmake --preset=linux-release-clang
cmake --build --preset=linux-release-clang
./build/linux-release-clang/launcher/liara_launcher
```

For Windows, or for the full setup procedure, see
[`docs/BOOTSTRAP.md`](./docs/BOOTSTRAP.md).

---

## Documentation

The project documentation is structured as separate documents, each
focused on one aspect:

| Document                                            | Purpose                                        |
|-----------------------------------------------------|------------------------------------------------|
| [`ARCHITECTURE.md`](./docs/ARCHITECTURE.md)         | Foundational design, philosophy, principles   |
| [`MODULES.md`](./docs/MODULES.md)                   | Concrete module decomposition and boundaries  |
| [`ROADMAP.md`](./docs/ROADMAP.md)                   | Phase 0 → v2.0 milestones with exit criteria  |
| [`CONTRIBUTING.md`](./docs/CONTRIBUTING.md)         | Daily workflow: branches, PRs, reviews        |
| [`CODE_STYLE.md`](./docs/CODE_STYLE.md)             | C++ conventions, clang-format, clang-tidy     |
| [`TOOLING.md`](./docs/TOOLING.md)                   | CI/CD, build, vcpkg, releases, docs           |
| [`BOOTSTRAP.md`](./docs/BOOTSTRAP.md)               | Setting up Arch and Windows for development   |

Interface design rules live in their own document, in the
`liara-interfaces` repository:
[`INTERFACES.md`](https://github.com/liara-engine/liara-interfaces/blob/main/INTERFACES.md).

User-facing documentation (tutorials, guides) is hosted at
[liara-engine.github.io](https://liara-engine.github.io) once Phase 0
is complete.

For newcomers, the recommended reading order is the sequence above,
top to bottom.

---

## Project Structure

The repositories under [`liara-engine`](https://github.com/liara-engine):

```
liara-engine/
├── liara                  # This repository: launcher, packaging,
│                          # documentation, compatibility matrix.
├── liara-interfaces       # C ABI headers shared by all modules.
├── liara-core             # ECS, math, assets, logger, settings.
├── liara-renderer         # Vulkan reference renderer.
├── liara-editor           # Editor application (post-v1.0).
├── liara-physics          # Physics module (post-v1.0).
├── docs-shared            # Shared documentation templates.
└── .github                # Organization-level workflows and templates.
```

The interfaces repository is the contract; everything else
implements or consumes it. See [`MODULES.md`](./docs/MODULES.md) for
the full picture.

---

## Roadmap at a Glance

The full roadmap is in [`ROADMAP.md`](./docs/ROADMAP.md). The
high-level shape:

- **Phase 0** — Infrastructure, repositories, CI, documentation.
- **v0.1** — "Hello Triangle" through the modular pipeline.
- **v0.2 — v0.5** — ECS, assets, lighting, input, audio.
- **v0.6** — Packaging for AUR and Windows.
- **v0.7 — v0.9** — Dogfooding: build a small game, fix the
  frictions discovered.
- **v1.0** — First shippable game milestone.
- **v1.x** — Editor, scripting, physics, advanced rendering.
- **v2.0** — Production-ready: an engine in which an ambitious
  project (think KSP-like or Plague Tale-like) is realistically
  attemptable by a determined developer.

The cadence is **variable, milestone-driven**, not calendar-based.
Pauses happen and are not failures.

---

## Technical Overview

A condensed summary of the choices documented in
[`ARCHITECTURE.md`](./docs/ARCHITECTURE.md) and
[`TOOLING.md`](./docs/TOOLING.md):

| Concern             | Choice                                             |
|---------------------|----------------------------------------------------|
| Language            | C++20 (interfaces in C ABI)                        |
| Graphics API        | Vulkan 1.3 via Vulkan-Hpp + VMA                    |
| Windowing           | SDL3                                               |
| UI                  | Dear ImGui                                         |
| Audio               | miniaudio                                          |
| Math                | Engine-internal types; GLM as private dep          |
| 3D format           | glTF 2.0                                           |
| Image formats       | PNG, JPG via stb_image                             |
| Configuration       | TOML (toml++)                                      |
| Build system        | CMake 3.29+ with presets                           |
| Dependency manager  | vcpkg in manifest mode                             |
| Test framework      | doctest                                            |
| ECS                 | Hand-written (sparse-set storage)                  |
| Documentation       | Doxygen (API) + mdBook (user)                      |
| CI/CD               | GitHub Actions with reusable workflows             |
| Static analysis     | clang-tidy + SonarCloud                            |
| Code formatting     | clang-format                                       |
| Release automation  | release-please                                     |
| Platforms           | Linux (Arch primary, Ubuntu CI), Windows 11        |
| Compilers           | GCC 14+, Clang 18+, MSVC 2022 17.6+                |

---

## Contributing

Contributions are welcome but optional. The project's primary mode is
solo development, and the workflow reflects that. If you would like
to contribute:

- Read [`CONTRIBUTING.md`](./docs/CONTRIBUTING.md) for the workflow.
- Open an issue before a substantial PR, so the change can be
  discussed before code is written.
- Small PRs (typo fixes, doc clarifications, obvious bug fixes) can
  be opened directly without prior discussion.

The project follows the [Contributor Covenant](https://www.contributor-covenant.org/)
code of conduct (a copy lives in `CODE_OF_CONDUCT.md`).

---

## License

Liara is released under the **MIT License**. See
[`LICENSE`](./LICENSE) for the full text. In short: do whatever you
want with the code, as long as you preserve the copyright notice.

The license applies uniformly across all repositories in the
`liara-engine` organization.

---

## Acknowledgments

Liara stands on the shoulders of others.

The architecture of this reboot owes much to **the lessons of the
previous Liara**, which made every mistake described in
`ARCHITECTURE.md`'s "Non-Goals" section. The reboot exists because
the original was educational.

The reference materials that made starting Vulkan possible:

- **[Vulkan Tutorial](https://vulkan-tutorial.com/)** by Alexander
  Overvoorde — the canonical entry point.
- **[Brendan Galea's Vulkan series](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)**
  — patient, well-explained, and the spine of the previous engine's
  early code.
- **[Sascha Willems' Vulkan samples](https://github.com/SaschaWillems/Vulkan)**
  — the place to look for "how do I do X in Vulkan".

The architectural patterns owe debt to:

- **Bevy** for the *extract-and-render* pattern that became
  Design 1 in this engine.
- **Godot** for *GDExtension*, the model for cross-language
  module interfaces.
- **Unreal Engine** for the *m_-prefixed PascalCase* style and the
  *editor-as-host* application model.
- **Hyprland** for the original (if imperfect) analogy of swappable
  components, even though Liara's modularity ended up working
  differently.

---

## Connect

- **Issues**: per repository on GitHub.
- **Discussions**: on this meta repository.
- **Documentation**: [liara-engine.github.io](https://liara-engine.github.io).

---

<div align="center">

*Built with care, modern C++, and probably too much coffee.* ☕
*A personal project, made open in case it is useful to anyone else.*

</div>
