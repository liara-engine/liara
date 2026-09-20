---
title: Commits and releases
description: The conventional-commit format, the scope vocabulary per repository, how release-please turns commits into tags, and what a release publishes.
sidebar:
  order: 6
---

## Conventional commits

Commit messages follow Conventional Commits 1.0:

```text
<type>(<scope>): <subject>

<body>

<footer>
```

The header is mandatory, the body and footer are optional, and lines wrap at 72 characters. The format is not decoration: release-please reads it to decide the next version number and to write the changelog, so a badly typed commit produces a wrong release rather than an untidy log.

### Types

| Type       | For                                                                       |
|------------|---------------------------------------------------------------------------|
| `feat`     | A new capability visible to a consumer                                    |
| `fix`      | A bug fix                                                                 |
| `perf`     | A performance improvement                                                 |
| `refactor` | A code change that is neither a feature nor a fix                         |
| `test`     | Adding or changing tests                                                  |
| `docs`     | Documentation only                                                        |
| `build`    | CMake, vcpkg, anything in the build                                       |
| `ci`       | CI configuration                                                          |
| `chore`    | Routine maintenance                                                       |
| `revert`   | Reverts an earlier commit                                                 |
| `style`    | Formatting. Rare, since clang-format removes most of the occasions for it |

### Scopes

A scope names a concern inside one repository, and it never names another module. A commit in `liara-core` scoped `assets` would be describing work that belongs in `liara-assets`, and the scope is the first place that mistake becomes visible.

| Repository         | Scopes                                                                           |
|--------------------|----------------------------------------------------------------------------------|
| `liara-interfaces` | `core`, `platform`, `renderer`, `assets`, `audio`, `physics`, `version`, `types` |
| `liara-core`       | `ecs`, `math`, `logger`, `settings`, `events`, `loop`                            |
| `liara-platform`   | `window`, `input`, `timing`, `signals`                                           |
| `liara-renderer`   | `device`, `swapchain`, `pipeline`, `passes`, `targets`, `imgui`, `debug`         |
| `liara-assets`     | `loading`, `decoding`, `lifetime`, `mesh`, `texture`                             |
| `liara-audio`      | `playback`, `mixing`, `sources`                                                  |
| `liara` (meta)     | `launcher`, `docs`, `packaging`, `scripts`                                       |

`build`, `ci` and `deps` work everywhere. The lists are open, and the rows for repositories that do not exist yet are there because the vocabulary is claimed with the namespace.

### Breaking changes

Both signals are required: a `!` after the type, and a `BREAKING CHANGE:` footer describing the impact.

```text
feat(renderer)!: change the packet structure

BREAKING CHANGE: liara_render_packet gains a field between
`meshes` and `camera`, so any consumer holding a compiled
copy of the old layout has to be rebuilt.
```

For a module that exposes an interface, which is most of them, that combination is what makes release-please bump the major version. It is also the only thing that will: a behavioral change behind an unchanged signature produces no ABI diff, so nothing detects it automatically and the footer is the whole mechanism.

### Validation

commitlint runs on every pull request, in its own workflow rather than as part of linting, and a non-conforming commit fails the checks. There is no Husky hook, because Husky needs Node in every developer's environment for a check CI already performs.

The same workflow validates schemas. It walks the repository for `manifest.json`, `modules-registry.json` and `version.json`, resolves the `$schema` each one declares, and runs ajv against it, with a distinct error for a file that declares no schema and one for a schema it cannot fetch. It then reads the version release-please is about to tag out of `.release-please-manifest.json` and fails if that version is absent from `manifest.json`, which is what keeps a release from shipping with a manifest that does not know about it.

## release-please

Each repository configures release-please in `release-please-config.json`, with `.release-please-manifest.json` tracking the current version. It maintains a release pull request that accumulates the conventional commits since the last tag; merging that pull request creates the tag, the changelog entry and the GitHub release.

Release cycles are independent. Nothing synchronizes versions between modules, and nothing is meant to.

During Phase 0 every repository ran with `bump-minor-pre-major` and `bump-patch-for-minor-pre-major` on, which shifts both levels down: a breaking change bumped the minor and a feature bumped the patch. That is the normal pre-1.0 reading of semver, and it is what kept every repository inside 0.x for the whole bootstrap.

From v0.1 those two options are off, because the policy in [Version numbers](../../roadmap/#version-numbers) asks for the opposite: a repository's first breaking change after Phase 0 takes it to 1.0.0. With the options still on, that same commit would produce 0.3.0 instead. Turning them off is what makes `feat!` mean `1.0.0`, `feat` mean a minor bump and `fix` mean a patch bump, which is plain semver and what every repository does from here on.

The 0.0.x lockstep rule of [ADR 0005](../../adr/0005-version-encoding-and-compatibility/) stays load-bearing for as long as any repository sits at 0.0.x, which today is `liara-platform` alone.

Each module's configuration also carries `extra-files`, so the release commit rewrites `$.metadata.latest` in `manifest.json` and `$.version` in `vcpkg.json` alongside the tag. Those two fields are therefore never edited by hand.

### Several packages in one repository

The meta repository is the only one releasing more than one thing. It declares three packages with `separate-pull-requests` enabled, so each accumulates its own release pull request and its own line in `.release-please-manifest.json`:

| Package                     | Tag form          |
|-----------------------------|-------------------|
| `.` (the repository itself) | `vX.Y.Z`          |
| `schemas`                   | `schemas-vX.Y.Z`  |
| `launcher`                  | `launcher-vX.Y.Z` |

The root package lists the others in `exclude-paths`, so a change to the schemas does not bump the engine's version. Two consequences reach CMake: each `CMakeLists.txt` reads its version out of `.release-please-manifest.json` instead of hard-coding it, and the launcher reads the `launcher` key rather than the root one.

### Changelogs

`CHANGELOG.md` is generated from the commits, in Keep a Changelog form, grouped by scope. It is reviewed inside the release pull request before merging. Editing it by hand is possible and almost never necessary, which is the return on writing the commit messages properly in the first place.

### Release artifacts

Merging a release pull request triggers a workflow that builds the module and attaches binaries to the GitHub release, so a released version and a set of downloadable binaries are the same thing. It assembles the workspace exactly as the build matrix does, with `install` in place of `ctest`: an artifact produced by a different build from the one CI validated is an artifact nobody validated.

Each module release publishes the compiled library for Linux under GCC, Linux under Clang and Windows under MSVC, in both static and shared form, along with the public headers, the `manifest.json` for that version, and a SHA-256 checksum file covering everything.

Names follow `liara-<module>-<version>-<platform>-<toolchain>-<linkage>.<ext>`, as in `liara-core-0.3.1-linux-clang-shared.tar.zst` or `liara-renderer-0.3.0-windows-msvc-static.zip`. The convention is stable because the composition tooling parses it, which makes any change to it a breaking change for that tooling.

These are per-module artifacts and not a bundled engine. Assembling a specific set of module versions into something runnable is the composition script's job, described in [Roadmap](../../roadmap/), and it consumes these artifacts rather than producing them.
