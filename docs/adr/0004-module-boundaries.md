---
title: "ADR 0004: What is a module, and what stays in the core"
description: The four kinds of repository, the substitution test that decides which is which, and why platform, assets and audio never start inside the core.
sidebar:
  label: "0004 · Module boundaries"
  order: 4
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-08-26 | Antoine  |

## Context

[ADR 0001](../0001-multi-repository-layout/) and [ADR 0002](../0002-c-abi-as-the-inter-module-contract/) say the project is split into modules with a hard contract between them. Neither says where the splits go. That question came up as soon as the module list had to be written down.

The first version of that list had `liara-core` containing the ECS, math, the logger, settings, the event bus, the loop, plus asset management, plus window and input, plus audio. Everything that wasn't rendering. It looked reasonable because each of those had a small implementation at the time, and none of them seemed worth a repository of its own.

That is exactly what happened in the previous project, and the reason it happened is that a core described as "the foundation" has no criterion for refusing anything. Every subsystem is foundational from some angle. Once assets and platform are inside it, the core depends on a file format library and a windowing library, cannot be built without either, and cannot be tested without both.

## Decision

Repositories fall into four kinds, and the kind decides what the repository is allowed to do.

| Kind           | Repositories                                                                                           | Allowed to                                                           |
|----------------|--------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------|
| Contract       | `liara-interfaces`                                                                                     | Nothing. Headers only, depends on nothing                            |
| Module         | `liara-core`, `liara-platform`, `liara-renderer`, `liara-assets`, `liara-audio`, later `liara-physics` | Implement one part of the contract, and depend on the contract alone |
| Host           | `liara` (the launcher), later `liara-editor`                                                           | Compose modules, and know the module graph, which nothing else may   |
| Infrastructure | `docs-shared`, `liara-docs`, `.github`                                                                 | Carry no engine code                                                 |

Three of the module repositories exist today: `liara-interfaces`, `liara-core` and `liara-renderer`. The other three are named here because the namespaces are claimed from the first line, not because the repositories are there.

The test for whether something is a module comes from [The rule that defines a module](../../modules/#the-rule-that-defines-a-module): could it be reimplemented in another language against the same C interface and substituted? If not, it is not a module and it does not get a repository.

Applying that test, `liara-core` keeps the ECS, math, the logger, settings, the event bus and the loop primitives. Platform, assets and audio become their own modules at the point where they exist, instead of starting in the core and being extracted afterwards.

## Alternatives considered

**Start with a large core and extract when it hurts.** The obvious approach: fewer repositories early, split when the pain is real, avoid guessing wrong. Rejected because "when it hurts" lands after every extraction has become expensive. A subsystem inside the core accumulates callers that reach into it directly, and by the time the split is obviously needed, the work is a rewrite. I have done the extract-later version and it did not get done.

**Keep the boundaries but ship them from one repository at first**, splitting the repositories later while keeping the interfaces. This one was seriously considered, and it is already what [One namespace per subsystem](../../architecture/modularity/#one-namespace-per-subsystem-from-the-first-line) does for ABI namespaces: every subsystem gets its own namespace from the first line, whichever repository implements it. Rejected as a general policy for the reason above, and because doing the split in two steps means doing the packaging work twice.

**Finer modules**, math separate from the ECS, the logger separate again. Rejected because none of them passes the substitution test in a way that means anything. Nobody reimplements a logger in Rust and swaps it in, and each additional repository has a fixed cost in CI, in releases and in documentation.

## Consequences

There are more repositories earlier than the amount of code justifies. `liara-platform` exists from v0.1 for one window and an event pump. That is accepted: the cost of a repository is mostly one-time setup, and the cost of not having one shows up later and is not one-time.

Some things become awkward on purpose. Assets and the renderer have to exchange data through the contract and through the host rather than by calling each other, so adding an interaction between them is a change to `liara-interfaces`, deliberately more expensive than a function call.

The core is now describable in one sentence, and I can tell whether something belongs in it without arguing with myself. That was the actual goal.

Dependencies sort themselves out, because each module declares only its own. That is easy to check today: `liara-core/vcpkg.json` declares no runtime dependency at all, with doctest sitting behind its `tests` feature, and there is no build in which the ECS drags in a glTF parser.

## Revisit if

- A subsystem inside `liara-core` starts being replaceable in a way that means something. The likeliest one is the logger, if the choice between spdlog and something in-house turns into a real configuration point.
- Two modules end up passing so much through the host that the indirection costs more than the separation is worth, with a profile to show it.
- A module turns out to be too small to justify its own release cycle after a version or two of living with it. Merging back is allowed, it just needs a record of its own.
