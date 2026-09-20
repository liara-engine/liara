---
title: About Liara
description: High-level overview of the Liara engine, its goals, and its structure.
sidebar:
    order: 0
---

# Liara Engine

A 3D game engine in modern C++ with Vulkan, built as a set of independently versioned modules that talk to each other through a C ABI.

It is a personal project, and a reboot of an earlier monolithic engine of mine. The reboot exists to apply what that one taught rather than to discard it.

## Why

Three reasons:

1. **Learning.** Modern graphics programming, Vulkan, modern C++, large-scale software architecture, build systems, CI/CD. Building a game engine touches all of these and forces good answers.
2. **Curiosity.** Wanting to understand how engines work, in detail, not just at the API level.
3. **Joy.** Some people garden; some people build engines. Productivity is not the metric here.

Liara does not aim to be the next Unity. It aims to be an engine that, by v1.0, lets a sufficiently motivated developer build and ship a small 3D game without feeling cheated.

## Quick start

Build the engine and run the demo (assuming you have Arch Linux with all required dependencies installed):

```bash
git clone https://github.com/liara-engine/liara.git
cd liara
./scripts/liara.sh setup --preset linux-release-clang
./scripts/liara.sh build --preset linux-release-clang
./scripts/liara.sh launch --preset linux-release-clang
```

For Windows, or for the full procedure from a machine that has none of the dependencies, see [Bootstrap](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/bootstrap/).

## Documentation

| Section                                                                                                       | For                                                                  |
|---------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------|
| [Architecture](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/architecture/) | The design and the reasoning behind it                               |
| [Modules](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/modules/)           | What each repository holds, and what crosses a boundary              |
| [Roadmap](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/roadmap/)           | Phase 0 through v2.0, with an exit criterion per milestone           |
| [Bootstrap](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/bootstrap/)       | Setting up Arch or Windows for development                           |
| [Contributing](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/contributing/) | Branches, pull requests, reviews                                     |
| [Code style](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/code-style/)     | The invariants, the shared baselines, and where a module may diverge |
| [Tooling](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/tooling/)           | Build, dependencies, CI, tests, releases                             |
| [Decision records](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/adr/)      | Major decisions, with what else was on the table                     |

The interface design rules live in the `liara-interfaces` repository, as [`INTERFACES.md`](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/INTERFACES/).

Everything is published at [liara-engine.liara-engine-documentation.workers.dev](https://liara-engine.liara-engine-documentation.workers.dev/).

User-facing documentation, meaning tutorials and guides rather than the engine's own design documents, can be found at TODO.

## Repositories

| Repository                                                             | Holds                                                                              |
|------------------------------------------------------------------------|------------------------------------------------------------------------------------|
| [`liara`](https://github.com/liara-engine/liara)                       | This one: the launcher, the workspace orchestrator, the documentation, the schemas |
| [`liara-interfaces`](https://github.com/liara-engine/liara-interfaces) | The C ABI headers every module implements or consumes                              |
| [`liara-core`](https://github.com/liara-engine/liara-core)             | The ECS, math, logger, settings, events, loop primitives                           |
| [`liara-renderer`](https://github.com/liara-engine/liara-renderer)     | The reference Vulkan renderer                                                      |
| [`docs-shared`](https://github.com/liara-engine/docs-shared)           | The Astro preset, the site tools, the documentation builder image                  |
| [`liara-docs`](https://github.com/liara-engine/liara-docs)             | Where the documentation is hosted, and the edge worker serving it                  |
| [`.github`](https://github.com/liara-engine/.github)                   | The reusable CI workflows every repository calls                                   |

`liara-platform`, `liara-assets`, `liara-audio`, `liara-physics` and `liara-editor` have their ABI namespaces claimed and no repository yet. Each is created when its first line of code is written.

## Roadmap at a glance

The full roadmap is in [`ROADMAP.md`](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/ROADMAP).
The high-level shape:

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

## State

v0.1, hello triangle. Phase 0 — the infrastructure rather than the engine — is closed. What exists today is the contract, three placeholder modules, a launcher that composes them and checks their ABI versions against each other, and the CI and documentation pipeline around all of it.

The roadmap is the honest account of the rest.

## Contributing

Contributions are welcome, but optional. The project is a personal one, and I will not be offended if you do not contribute. If you would like to contribute:

1. Read the [contributing guide](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/contributing/).
2. Open an issue before a substantial PR, so the change can be discussed before code is written. Small PRs (typo fixes, doc clarifications, obvious bug fixes) can be opened directly without prior discussion.
3. Fork the repository, make your changes in a branch, and open a pull request against `main`. The PR will be reviewed and merged if it is acceptable.

This project follows the [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/version/3/0/code_of_conduct/). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers. A copy of the code of conduct can be found in the [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file.

## License

Liara is released under the [MIT License](https://opensource.org/license/mit/). See the [LICENSE](LICENSE) file for details. In short: do whatever you want with the code, as long as you preserve the copyright notice.

The license applies uniformly across all repositories in the `liara-engine` organization, and to all code in the `liara` repository, including the launcher, the documentation, the schemas, and the CI workflows. The license does not apply to third-party dependencies, which are under their own licenses.

## Acknowledgements

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

## Connect

- **Issues**: per repository on GitHub.
- **Discussions**: on this meta repository.
- **Documentation**: [liara-engine.liara-engine-documentation.workers.dev](https://liara-engine.liara-engine-documentation.workers.dev/)

---

<div align="center">

*Built with care, modern C++, and probably too much coffee.* ☕
*A personal project, made open in case it is useful to anyone else.*

</div>
