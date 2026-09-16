---
title: Static analysis
description: clang-format and clang-tidy as a shared baseline each module may refine, what CI enforces, and why SonarCloud is not switched on.
sidebar:
  order: 5
---

## A baseline, not a rulebook

Both tools are configured per repository, in a `.clang-format` and a `.clang-tidy` at the root, and both start from the same baseline. A module may refine its copy where its own material justifies it. Forbidding magic numbers reads well in most of the project and produces nothing but noise in the renderer, where the constants are Vulkan's.

Two things survive that freedom. Formatting is mechanical and never manual, so every repository has a configuration and CI enforces it; a formatting argument in a review is a bug in the configuration rather than in the pull request. And a check a module removes from its baseline carries a comment above it saying what noise it produced, in the file, for whoever wonders six months later whether it can go back on.

The baselines themselves, with the reasoning for each choice, are in [`code-style.md`](../code-style/).

## clang-format

clang-format 20 or newer. CI runs `clang-format --dry-run --Werror` over every tracked C and C++ source of the calling repository, and any deviation fails the build. Generated files are excluded by pattern, because reformatting a file that is compared byte-for-byte against its generator breaks the check that produced it.

There is no optional pre-commit hook. `./scripts/liara.sh check --fix` applies the formatting locally for anyone who wants it before pushing, and CI stays the authority.

`liara-interfaces` is the one repository whose configuration diverges structurally rather than by preference: its public headers are C, so it formats them as C.

## clang-tidy

clang-tidy 20 or newer, run in the integration tier against the workspace's compilation database, over every tracked translation unit of the repository under test minus the generated-file exclusions. The baseline enables whole families (`bugprone-*`, `cert-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, `portability-*`, `readability-*` and more) with a short list of subtractions, rather than turning on `*`.

`WarningsAsErrors` is set to `*` with four exceptions, which means the practical gate is zero warnings on the repository, not zero new ones. A warning in code the pull request never touched still fails it.

The naming rules in `CheckOptions` are where the C boundary is enforced mechanically: `liara_`-prefixed lowercase structs, enums, unions and functions, `_t`-suffixed typedefs, `LIARA_`-prefixed uppercase macros and enum constants. Those identifiers are the ABI, so renaming one is a breaking change for every consumer in every language, and clang-tidy is what notices.

## SonarCloud

Deferred. The analysis workflow is not written and no quality gate is configured, so nothing in CI consults it today. It would add a third layer over clang-tidy, tracking trends and duplication across time rather than judging one build, and it is free for open-source projects, which is why it is still on the list rather than off it.

When it arrives it judges the diff and not the existing code, for the same reason coverage waits for v0.2: a gate that fires on everything already written gets switched off within a week.
