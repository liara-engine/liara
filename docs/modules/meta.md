---
title: "liara: the meta repository"
description: The launcher, the workspace orchestrator, the documentation and the schemas. Everything that has to know about every module at once.
sidebar:
    label: "liara (meta)"
    order: 1
---

The meta repository is the public face of the project and the orchestrator of everything else. It holds no engine code, and it holds the things that need to know about all modules at once.

## What it is for

It is the **landing page**: the README a visitor reads first, the issue tracker that catches general questions, the discussions board for project-wide topics.

It is the **host**: `launcher/` is the small executable that creates each module, negotiates ABI versions, wires them together and runs the loop. It is also what will ship in the AUR package.

It is the **workspace orchestrator**: `scripts/liara.py` and its two wrappers clone every other repository into `workspace/`, generate the superbuild and the presets, and configure the build.

And it is the **compatibility record**, in the sense that its `manifest.json` declares what the launcher requires and every module's manifest declares what it provides. There is no separate matrix file, and [ADR 0006](../adr/0006-manifest-as-compatibility-source-of-truth/) says why.

## Contents

| Path            | Holds                                                                                                                                                    |
|-----------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `docs/`         | These pages, the other foundational documents, and the ADRs                                                                                              |
| `launcher/`     | The host application: `main.cpp`, its `config.h.in`, its `CMakeLists.txt`                                                                                |
| `scripts/`      | `liara.py`, the `liara.sh` and `liara.ps1` wrappers, and the `CMakeLists.txt` and `CMakePresets.json` templates the workspace is generated from          |
| `schemas/`      | The JSON schemas for `manifest.json`, the modules registry and the version file, served over GitHub Pages so that a `$schema` URL resolves               |
| `workspace/`    | Where `liara.sh setup` clones the module repositories. Not tracked                                                                                       |
| `manifest.json` | This repository's own manifest, in v2 form, with an `artifacts` block declaring the launcher's ABI requirement separately from the repository's versions |

`packaging/`, holding the `PKGBUILD` and the Windows packaging script, arrives with v0.6.

The user guide arrives with v0.1, for the reason that writing tutorials against provisional code is wasted work. Its most likely home is a directory in this repository rather than a repository of its own, and that is not settled. Whatever shape it takes in Git, it is published as a section of the documentation site in its own right, alongside the modules, which is what the `/user/` path reserved in the hub already points at.

## What it does not hold

No engine logic, no rendering, no ECS. Nothing that belongs in a module.

No generated API reference either. Each module's Doxygen XML is turned into pages by its own documentation build and published to `liara-docs`, which is described in `docs-shared`<!-- TODO link → the documentation-pipeline page of docs-shared -->.

And no Dockerfile. The documentation builder image used to live here in a `docker/` directory and now lives in `docs-shared`, which is the repository the image is built from. One trace of the move is still in the code: the root package of `release-please-config.json` lists `docker/**` in its `exclude-paths` for a directory that no longer exists.

## Versioning

The meta repository versions along the milestone roadmap rather than with any module, and module repositories version independently of it and of each other.

It is also the only repository releasing more than one thing: the repository itself on plain `vX.Y.Z` tags, plus `schemas` and `launcher` on component-prefixed ones. The mechanics are in [Commits and releases](../tooling/commits-and-releases/#several-packages-in-one-repository).
