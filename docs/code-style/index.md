---
title: Code style
description: Three tiers of rule, six invariants nothing adapts, and the deliberate decision not to track what each module does differently.
sidebar:
  label: Overview
  order: 0
---

These pages cover the C++ implementation behind the interfaces, plus the build rules that apply everywhere. The style of the C interfaces themselves is in [Naming](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/naming/) and [Types](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/types/), which win over anything here.

## Why there are rules at all

Style rules exist to remove decisions that do not matter, so that attention goes to the ones that do. Two developers arguing about brace placement are two developers not solving the problem.

That has a consequence people usually skip: a rule that has to be maintained is itself a decision that keeps coming back. A style document mirroring every line of every configuration file across every repository becomes a second source of truth, it drifts from the first, and every drift is another small decision to re-make. So these pages deliberately do not do that.

## Three tiers

**Invariants** hold in every repository, forever, because breaking one breaks something real: an ABI, a build, a consumer. They are stated here, argued for here, and no module opts out. There are six, and being few is the point.

**Baselines** are the shared `.clang-format` and `.clang-tidy`. Every repository is created from them, and they are the default answer to how something should look. They are reproduced here in outline, with the reasoning behind the choices that are not obvious from the option name.

**Local adaptations** are a module's deviations from its baseline. They are expected, they are legitimate, and they are not tracked here. A renderer disabling the magic-number check because `vec3(1, 0, 0)` is not a magic number is not violating the style, it is applying the style's own reasoning to its own material.

The practical test when in doubt: would another module have to know about this? If yes it is an invariant and belongs here. If no it belongs in that module's configuration file, with a comment saying why.

Where these pages and a tool's default disagree, these pages win and the tool gets configured. Where these pages and a module's configuration disagree about something that is not an invariant, the file wins, and this is where you come to find out which of the two situations you are in.

## The six invariants

**1. The C boundary follows `INTERFACES.md`, not this.** Lowercase underscore-separated identifiers, module prefix, `_t` suffix on typedefs. Non-negotiable because those identifiers are the ABI: renaming one is a breaking change for every consumer, in every language.

**2. The C++ implementation is invisible from outside.** Whatever a module does internally, no other module sees it. That is what makes local adaptation safe in the first place.

**3. Nothing that is not plain data crosses the boundary.** No exception, no `std::` type, no C++ object, no ownership implied by a raw pointer without a documented `destroy`.

**4. Formatting is mechanical, never manual.** Every repository has a `.clang-format`, CI runs it with `--dry-run --Werror`, and a formatting discussion in a review is a bug in the configuration rather than in the pull request. The rules a module picks are its own; having them and enforcing them is not optional.

**5. A disabled check carries its reason, in the file.** Any check a module removes from its baseline gets a comment above it saying what noise it produced. Not bureaucracy: it is for the person who six months later wonders whether it can go back on, and it is the mechanism that lets these pages stop tracking the diffs.

**6. Ownership is explicit, and raw ownership lives only at the boundary.** Inside a module, ownership is smart pointers and RAII. The one exception is the `create` and `destroy` pair of the C shim, where the handle's lifetime is the caller's and no smart pointer can cross. [Memory](./memory/) has the detail.

## Two regimes

The project runs two style regimes, separated by the C interface boundary.

**At the boundary**, `INTERFACES.md` applies: lowercase underscore-separated identifiers, prefixed by module. That style is non-negotiable and it is the convention of the wider C ecosystem, which Vulkan, SDL, libcurl and POSIX all share.

**Inside a module's C++ implementation**, these pages apply: CamelCase for types and methods, `m_`-prefixed members, and the rest of [Naming](./naming/). It is internal to each module and no other module ever sees it.

The transition happens in the shim, which is the one file where the two meet. [File organization](./files/) shows what it looks like.
