---
title: Architecture decision records
description: What a record holds, how the format differs from the plain Nygard template, and the index of the ten written so far.
sidebar:
    label: Overview
    order: 0
---

Each file here records one decision: what was decided, what else was on the table, and what the decision costs.

Records are append-only. A decision that stops holding is not edited and not deleted. A new record supersedes it, and the old one has its status changed to `Superseded by NNNN`. Keeping the reasoning that turned out to be wrong is the point of the exercise, because it says what the alternatives looked like at the time, which is the part nobody can reconstruct afterwards.

Most of the Phase 0 records are retrospective: the decision was taken while bootstrapping and written down later, and each one says so under its metadata table. A retrospective record makes a narrower claim than an original one. It says why the decision still holds today, and it does not try to reconstruct the state of mind that produced it.

## The format

[`contributing.md`](../contributing/) §11.2 gives the plain Michael Nygard template: context, decision, consequences. The records here add two sections to it, and those two carry most of what makes them worth re-reading.

**Alternatives considered** names every option that was on the table and what killed it. An alternatives section that lists options without saying why each one lost is no help at all to whoever reopens the question two years later.

**Revisit if** lists the observations that would make the decision worth reopening. Each item is written so it can be checked rather than felt: a measurement on a hot path, a normal change that routinely costs four pull requests, a contributor who is not me.

Every record opens with a table giving its status, its date, and who decided. Status is one of `Proposed`, `Accepted`, `Deprecated`, or `Superseded by NNNN`.

## The records

| ADR                                                       | Title                                                       | Status   |
|-----------------------------------------------------------|-------------------------------------------------------------|----------|
| [0001](./0001-multi-repository-layout/)                   | Multi-repository layout                                     | Accepted |
| [0002](./0002-c-abi-as-the-inter-module-contract/)        | A C ABI as the inter-module contract                        | Accepted |
| [0003](./0003-the-host-composes-modules/)                 | The host composes modules                                   | Accepted |
| [0004](./0004-module-boundaries/)                         | What is a module, and what stays in the core                | Accepted |
| [0005](./0005-version-encoding-and-compatibility/)        | Version encoding and compatibility semantics                | Accepted |
| [0006](./0006-manifest-as-compatibility-source-of-truth/) | `manifest.json` as the single compatibility source of truth | Accepted |
| [0007](./0007-cmake-presets-and-vcpkg/)                   | CMake with presets, and vcpkg in manifest mode              | Accepted |
| [0008](./0008-two-tier-ci/)                               | Two-tier CI, and reusable workflows                         | Accepted |
| [0009](./0009-in-house-ecs/)                              | An in-house ECS rather than an existing library             | Proposed |
| [0010](./0010-vulkan-without-abstraction-layer/)          | Vulkan directly, without a rendering abstraction layer      | Proposed |

New tooling requires a record of its own (`TOOLING.md` §1<!-- TODO link → the tooling inventory page, once tooling is split into a directory -->). The one exception is the bootstrapping phase, whose policy is in [`roadmap.md`](../roadmap/) §4.
