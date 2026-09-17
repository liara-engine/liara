---
title: Proposing a decision
description: When a change needs a record of its own, how one is opened, and the phase where none of this applies.
sidebar:
  order: 5
---

## When one is required

A change needs a decision record when it introduces a new tool to the project, meaning a new dependency, a new build step or a third-party library beyond the existing list. When it modifies a principle in [Architecture](../architecture/). When it reverses an earlier record. Or when its consequences reach many parts of the project rather than one module's internals.

A bug fix does not. Neither does refactoring inside a module, nor adding a feature within an established subsystem.

When it is unclear, write one. They are cheap to produce, and the context they hold is expensive to recover once it is gone.

The one exception is the bootstrapping phase, where the project is still forming its architecture and subjecting that formation to its own output would be circular. [Phase 0](../roadmap/phase-0/) states what the exemption does and does not cover.

## The format

[Architecture decision records](../adr/) describes the format these records actually use, which adds two sections to the plain Michael Nygard template and is where the value of re-reading one usually sits.

Records are numbered sequentially from `0001`, and they are write-once. A reversed decision becomes a new record superseding the old one rather than an edit to it.

## The workflow

A record at proposal stage is opened as a pull request with its status set to `Proposed`, and the pull request's discussion is where the decision is argued. On merge the status becomes `Accepted` and the decision takes effect.
