---
title: Continuous integration
description: The two CI tiers and what each one proves, the nine-leg matrix, where the workflows live, and which checks block a merge.
sidebar:
  order: 3
---

## Two tiers

CI answers two questions that are not the same question, so it runs as two pipelines with different build models. [ADR 0008](../../adr/0008-two-tier-ci/) records why.

The **branch tier** runs on every push outside `main`. It builds the repository on its own, on one Linux preset, against the `liara-interfaces` version that repository's own manifest declares, and runs clang-format. It fetches the preset template from the meta repository rather than carrying a copy. It is not a required check, and it is deliberately incomplete: no Windows, no shared libraries, no clang-tidy, no composition check. A green branch build means the repository compiles, and nothing beyond that.

The **integration tier** runs on pull requests, on pushes to `main`, on tags matching `vX.Y.Z`, and on manual dispatch. It assembles the workspace superbuild with the repository under test substituted for its clone, builds the full matrix, and runs clang-tidy against the workspace's compilation database. This is what branch protection requires.

Every step of both tiers writes a report fragment, and a final job renders them into one sticky pull request comment. Its budget is finite, since nine legs of build logs would overflow GitHub's comment limit, so a passing leg gets one line and only failures carry their output.

## The build matrix

The matrix is expressed as preset names rather than as (OS, compiler, build type) tuples, because the presets are the project's supported build interface. A leg CI can run but a developer cannot reproduce with one preset name is a leg that lies about what it proves.

| Leg                    | Runner       | Configure preset            | Requires Zig and Rust |
|------------------------|--------------|-----------------------------|-----------------------|
| `linux-gcc-debug`      | ubuntu-24.04 | `linux-debug-gcc`           | yes                   |
| `linux-gcc-release`    | ubuntu-24.04 | `linux-release-gcc`         | no                    |
| `linux-clang-debug`    | ubuntu-24.04 | `linux-debug-clang`         | yes                   |
| `linux-clang-release`  | ubuntu-24.04 | `linux-release-clang`       | no                    |
| `linux-shared-link`    | ubuntu-24.04 | `linux-debug-clang-link`    | no                    |
| `linux-shared-runtime` | ubuntu-24.04 | `linux-debug-clang-runtime` | no                    |
| `windows-debug`        | windows-2022 | `windows`                   | no                    |
| `windows-release`      | windows-2022 | `windows`                   | no                    |
| `windows-runtime`      | windows-2022 | `windows-runtime`           | no                    |

A repository with nothing to build narrows the matrix by passing its own JSON array.

The two legs marked in the last column configure with `LIARA_INTERFACES_REQUIRE_CROSS_LANGUAGE_TESTS=ON`, which makes the Zig and Rust tests mandatory rather than conditional on `find_program` finding a toolchain. Without that, "installed nowhere" and "installed everywhere" would look identical from outside.

The matrix runs on Ubuntu rather than Arch because GitHub-hosted runners do not offer Arch and a self-hosted one would be operational work with no owner. What backs the claim that the project supports Arch is that it is the machine the engine is developed on daily, and CI treats Ubuntu as a reasonable stand-in for a modern Linux.

## Parallelism and caching

Each leg runs independently, and the caching is aggressive in one place and absent in another.

The **vcpkg binary cache** is a per-leg GitHub Actions cache over `VCPKG_DEFAULT_BINARY_CACHE`, keyed by the runner OS, the preset, and a hash of the merged manifest, so it invalidates itself when any of those change.

A **compile cache** is not installed. The workspace `CMakeLists.txt` will use sccache or ccache when it finds one, but CI does not provide either: with the current dependency set the build is dominated by the project's own handful of translation units, and the cache transfer would cost more than it saves. That is worth revisiting when a leg exceeds a few minutes.

With warm caches a leg finishes in under five minutes, and the whole matrix, in parallel, in under ten.

## What blocks a merge

- Every matrix leg passes, `linux-shared-link` and `linux-shared-runtime` included.
- clang-format produces no diff.
- clang-tidy produces no warnings.
- The test suite passes, cross-language tests included.
- The ABI checks pass, for `liara-interfaces` only.

Two gates exist on paper and are not switched on. **Coverage** is deferred to v0.2: measuring coverage of Phase 0's placeholder implementations would report a number about scaffolding rather than about the engine. The **SonarCloud quality gate** is deferred as well, and the analysis workflow is not yet written.

Documentation generation runs but does not block, because a documentation failure should not stop a code merge.

## Where the workflows live

Six repositories run substantially the same pipelines, so the YAML lives once, in the organization's `.github` repository, as reusable workflows called by a thin caller in each repository. Copy-pasting would mean a fix to one repository's CI never reaching the other five, with the drift invisible until two pipelines that were supposed to be identical behave differently. Composite actions cannot express a matrix or multiple jobs, which is most of what these pipelines are, so they are used for the step sequences shared between workflows and not for the pipelines themselves.

Each module repository keeps small caller files in `.github/workflows/` (build, lint, docs, commitlint, release) that do nothing but configure inputs.

### The catalog

**Build and quality.** `reusable-build-and-test.yml` assembles the workspace superbuild around the calling repository and runs the matrix above; it takes the repository name, the matrix as a JSON array, and the CTest labels to exclude. `reusable-lint.yml` runs clang-format over the calling repository's tracked sources and clang-tidy over its translation units against the workspace's compilation database, excluding generated files by pattern, because reformatting a file that is compared byte-for-byte against its generator breaks the check that produced it.

There is no separate shared-library workflow. The `-link` and `-runtime` legs already build every module as a shared library and, for `-runtime`, make the launcher resolve them through `dlopen` or `LoadLibrary`, so a dedicated workflow would duplicate them under another name and drift.

**Interface integrity**, in `liara-interfaces` only. `reusable-abi-header-portability.yml` compiles every public header standalone and warning-free as C and C++ across several language standards. `reusable-abi-interface-rules.yml` enforces the naming and shape rules of `INTERFACES.md`<!-- TODO link → the interface rules page, once interfaces is split into a directory -->. `reusable-abi-layout-freeze.yml` freezes struct layouts against a generated golden header. `reusable-abi-snapshot.yml` snapshots the ABI surface and diffs it against the pull request base to compute the required version bump. `reusable-abi-report.yml` aggregates the four into one sticky comment.

**Documentation.** `reusable-build-docs.yml` and `reusable-deploy-docs.yml` build the docs in the builder image and publish them to `liara-docs`, composed by `reusable-docs.yml`. `reusable-docs-preview.yml` builds a per-pull-request preview and posts its URL. `reusable-docs-preview-cleanup.yml` removes that directory when the pull request closes.

**Release and housekeeping.** `reusable-commitlint.yml` validates conventional-commit format. `reusable-validate-manifest.yml` validates each `manifest.json` against its schema and checks that the version release-please is about to tag exists in it. `reusable-generate-docker.yml` builds and pushes the documentation builder image. `reusable-clean-ghcr.yml` prunes untagged container images on a weekly schedule, keeping the most recent ones.

release-please itself is invoked directly from each repository rather than through a reusable workflow.

### Pinning a version

Consuming workflows reference the reusable workflows by tag and never by branch. Every release of  the `.github` repository force-updates a `vX` and a `vX.Y` tag onto the new release and creates a `vX.Y.Z` tag, with major, minor and patch decided by release-please from the conventional commits.

Pin to `@vX` unless you have a reason not to: fixes and features arrive on their own, and a change that breaks the interface forces an explicit bump in the caller. Workflows that additionally check out the `.github` repository to reach a shared script take a `github-repo-version` input for the same purpose, and callers pass it the same ref they pinned the workflow to. The two must not drift, or a workflow runs against a script from a different release.

Preview tags named `preview-<pr-number>` are force-updated on every push to a pull request, so a workflow under review can be called by its preview tag before it is released.
