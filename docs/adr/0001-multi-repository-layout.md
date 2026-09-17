---
title: "ADR 0001: Multi-repository layout"
description: Why each module lives in its own Git repository, what the previous single-repository engine cost, and what the split costs in return.
sidebar:
    label: "0001 · Multi-repository layout"
    order: 1
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-08-26 | Antoine  |

*Retrospective.*

## Context

Liara is a reboot of an earlier engine of mine, which lived in a single repository. What went wrong there is the whole context for this decision, so here is what it actually looked like.

Everything being in one place meant everything could reach everything. Nothing stopped me from coupling two systems together, so I did, repeatedly, because in the moment it was always the fastest way to get the thing working. The coupling then made later tasks harder than they should have been, and tasks that are harder than they should be are tasks I put off. Several parts of that project stalled for that reason and not for any technical one.

Opening the project to work on one thing meant having every other thing in front of me. I would open it to work on X and spend the day on everything except X. That is not a discipline problem I can solve by trying harder, and pretending otherwise is how the previous project ended.

There was also a mismatch I never got used to. The design talked about interchangeable subsystems that don't know about each other, and the folder structure said they all live together and can call each other freely. Reading the docs and reading the tree gave two different answers about what the project was.

## Decision

One Git repository per module, under the `liara-engine` organization. A module depends on `liara-interfaces` and on nothing else. Local development happens in a workspace assembled by `scripts/liara.py` (wrapped by `liara.sh` on Linux and `liara.ps1` on Windows), which clones the repositories side by side under `workspace/` and generates a top-level `CMakeLists.txt` that builds them as one superbuild.

## Alternatives considered

**A monorepo.** Easier on every operational axis, and I do not dispute that. Cross-cutting refactors happen in one commit instead of five coordinated pull requests. There is no compatibility matrix to maintain, no workspace to bootstrap, no version negotiation between components that ship together anyway. If the only criterion were which layout is less work to run, this would be the answer.

I rejected it because I have already built the monorepo version of this project and I know how it ends. The operational cost of the multi-repo layout is real, but it is bounded and mostly automated. The cost of the monorepo showed up as coupling I couldn't undo and tasks I couldn't start.

**A monorepo with enforced boundaries**: separate CMake targets, a lint rule against cross-module includes, a directory convention. This is the option I would recommend to most people, and it is the one I trust least for myself. A boundary I can cross by editing one line is a boundary I will cross at some point, with a good reason, at 2am. A boundary that requires opening another repository, changing the contract, releasing it and updating a manifest is one I will not cross by accident. The friction is doing the work here.

**A middle layout**, one repository for the engine and one per optional module. Rejected because the split would have to be decided up front and would inevitably be wrong. Whichever subsystems started out together would grow into each other, exactly as they did before, and separating them later means a breaking ABI change and a migration.

## Consequences

A change that crosses modules is several pull requests in a defined order, and I have to write down which versions work together. [Changing more than one repository](../../contributing/cross-repository/) describes the procedure. It is slow on purpose, since the friction is what keeps interface changes from being casual, but it is also genuinely slow, and on a day where I want to move fast it is the thing standing in the way.

Local development needs tooling that would not otherwise exist: a bootstrap script, a generated superbuild, a preset template shared across repositories, and a CI that assembles the workspace around whichever repository is under test. That is a real amount of Phase 0 work that a monorepo would not have required at all.

Each module can be built, tested and released alone, and a module's test suite has nothing to stand up but itself. This is the payoff, and it arrived on the first day rather than at some milestone.

The structure now matches what the documentation says. When I read that modules don't know about each other, the tree agrees, and I don't have to hold two versions of the project in my head.

## Revisit if

- The cross-repository coordination cost stops being bounded and starts dominating. If a normal change routinely takes four pull requests, the boundaries are in the wrong place, which is a  problem with [ADR 0004](../0004-module-boundaries/) rather than with this one.
- Contributors other than me appear in numbers, at which point the barrier to a first contribution matters more than my own working constraints do.
