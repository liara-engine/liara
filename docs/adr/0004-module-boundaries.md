# ADR 0004 — Module Boundaries: What Is a Module and What Stays in the Core

- **Status**: Accepted
- **Date**: 2026-08-26
- **Deciders**: Antoine

## Context

ADRs 0001 and 0002 say the project is split into modules with a hard contract between them. Neither says where the splits go. That question came up as soon as the module list had to be written down.

The first version of the list had `liara-core` containing the ECS, math, the logger, settings, the event bus, the loop, plus asset management, plus window and input, plus audio. Everything that wasn't rendering. It looked reasonable because each of those had a small implementation at the time, and none of them seemed worth a repository of its own.

That is exactly what happened in the previous project, and the reason it happened is that a core described as "the foundation" has no criterion for refusing anything. Every subsystem is foundational from some angle. Once assets and platform are inside it, the core depends on a file format library and a windowing library, cannot be built without either, and cannot be tested without both.

## Decision

Repositories fall into four kinds, and the kind determines what the repository is allowed to do:

- **Contract** — `liara-interfaces`. Headers only. Depends on nothing.
- **Modules** — `liara-core`, `liara-platform`, `liara-renderer`, `liara-assets`, `liara-audio`, and later `liara-physics`. Each implements one part of the contract and depends only on the contract.
- **Hosts** — `liara` (the launcher), later `liara-editor`. Compose modules. Allowed to know the module graph, which nothing else is.
- **Infrastructure** — `docs-shared`, `liara-docs`, `.github`. No engine code.

The test for whether something is a module, from `MODULES.md` §1.5: could it be reimplemented in another language against the same C interface and substituted? If not, it is not a module and does not get a repository.

Applying that test, `liara-core` keeps the ECS, math, the logger, settings, the event bus and loop primitives. Platform, assets and audio are their own modules from the point at which they exist, rather than starting in the core and being extracted later.

## Alternatives Considered

**Start with a large core and extract when it hurts.** The obvious approach: fewer repositories early, split when the pain is real, and avoid guessing wrong. Rejected because "when it hurts" is after every extraction has become expensive. A subsystem inside the core accumulates callers that reach into it directly, and by the time the split is obviously needed it is not a move any more, it is a rewrite. I have done the extract-later version and it did not get done.

**Keep the boundaries but ship them from one repository at first**, splitting the repositories later while keeping the interfaces. This was seriously considered, and it is what `ARCHITECTURE.md` §4.3 already does for ABI namespaces — every subsystem gets its own namespace from the first line regardless of which repository implements it. Rejected as a general policy for the same reason as above, and because the two-step version means doing the packaging work twice.

**Finer modules** — math separate from the ECS, the logger separate again. Rejected: none of them passes the substitution test in a way that means anything. Nobody reimplements a logger in Rust and swaps it in, and each additional repository has a fixed cost in CI, releases and documentation.

## Consequences

There are more repositories earlier than the amount of code justifies. `liara-platform` exists from v0.1 for one window and an event pump. That is accepted: the cost of a repository is mostly one-time setup, and the cost of not having one shows up later and is not one-time.

Some things become awkward on purpose. Assets and the renderer have to exchange data through the contract and through the host rather than by calling each other, so adding an interaction between them is a change to `liara-interfaces` — deliberately more expensive than a function call.

The core is now describable in one sentence, and I can tell whether something belongs in it without arguing with myself. That was the actual goal.

Dependencies sort themselves out. Because each module declares only its own, `liara-core` needs toml++ and glm and nothing else, and there is no build in which the ECS drags in a glTF parser.

## Revisit If

- A subsystem inside `liara-core` starts being replaceable in a way that means something — most likely the logger, if the choice between spdlog and something in-house turns into a real configuration point.
- Two modules end up passing so much through the host that the indirection costs more than the separation is worth. Measured, not suspected.
- A module turns out to be too small to justify its own release cycle after a version or two of living with it. Merging back is allowed; it just needs its own ADR.
