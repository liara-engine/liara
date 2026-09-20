---
title: Static analysis baselines
description: Two clang-tidy baselines, why each disabled check is disabled, and why naming is reported rather than enforced.
sidebar:
  order: 2
---

What CI actually runs, and when, is in [Static analysis](../../tooling/static-analysis/). This page is the baselines themselves and the reasoning behind them.

## Two of them

**The C++ baseline**, used by every repository holding C++, enables broad families (`bugprone-*`, `cert-*`, `clang-analyzer-*`, `concurrency-*`, `cppcoreguidelines-*`, `misc-*`, `modernize-*`, `performance-*`, `portability-*`, `readability-*`) and then subtracts the ones that produce noise in this project's material. It carries a `CheckOptions` block encoding the [naming table](../naming/).

**The C baseline**, used by `liara-interfaces`, starts from `-*` and adds back the same families minus everything that assumes C++: `modernize-use-using`, `use-nullptr`, `use-override`, `use-equals-default`, `concat-nested-namespaces`, and the whole `cppcoreguidelines-*` family. Its `CheckOptions` encode the C naming rules of `INTERFACES.md`: the `liara_` prefix, the `_t` suffix, and `LIARA_` plus `UPPER_CASE` for macros and enum constants.

## Naming is reported, not enforced

In both baselines, `readability-identifier-naming` is deliberately excluded from `WarningsAsErrors`. It runs, it reports, and it does not fail a build.

That is considered rather than forgotten. A naming violation is never silent corruption, since it is visible in the diff, and the check has enough edge cases that making it fatal would mean scattering `NOLINT` to satisfy a tool rather than a reader. The obvious edge case is the shim: C-named functions defined in a C++ translation unit violate the C++ options by construction.

Everything else in the baseline is fatal, because everything else catches something a reviewer plausibly misses.

## Why each check is off

`bugprone-easily-swappable-parameters` produces too many false positives in geometric code, where `(x, y, z)` parameters have an order inherent to the maths.

`bugprone-narrowing-conversions` fires constantly at the renderer and GPU boundary, where uploading double-precision world coordinates as floats is the intended behavior. Adding explicit casts everywhere would be noise with no bug caught.

`modernize-use-trailing-return-type` asks for `auto foo() -> int`, which is a stylistic choice nobody agrees on, and consistency with the wider C++ idiom wins.

`modernize-avoid-c-arrays` conflicts with the boundary: a plain-data struct crossing to C has to use a C array and cannot use `std::array`.

`cppcoreguidelines-non-private-member-variables-in-classes` and its `misc` counterpart do not distinguish a plain-data struct from a real class, and public members are the right tool for the first.

`portability-avoid-pragma-once` is off in **both** baselines, and it is the one disabled check that is not about noise. It argues for include guards; [Files](../files/#pragma-once) states `#pragma once` as the convention. The convention wins, so the check is subtracted once in each baseline rather than suppressed in every header the project owns.

Three more are off in the **C baseline only**, and all three are the same mistake: a check applying a C++ judgment to a C contract.

`modernize-redundant-void-arg` reads `liara_core_info(void)` as a redundant `void`. In C it is the opposite of redundant: `f()` declares a function with *unspecified* arguments and `f(void)` declares one with none, so removing it would widen every entry point in the contract.

`modernize-macro-to-enum` wants the version and layout macros to become enum constants. They cannot. They are used in constant expressions from C — a `static_assert`, a case label, an array size, which [Version encoding](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/versioning/) is explicit about — and an enum would add a type to the ABI surface where a macro adds nothing.

`performance-enum-size` suggests a smaller underlying type for `liara_result_codes` and friends. An enum's underlying type is part of the ABI layout: shrinking one changes every struct containing it and breaks every consumer holding a compiled copy. It stays enabled in the C++ baseline, where a module's internal enums are its own business.

`cppcoreguidelines-avoid-magic-numbers` and the matching readability check deserve a note of their own, because they are the clearest illustration of what this page is about. They are off in the baseline because graphics and maths code is full of constants meaningful as written: `vec3(1, 0, 0)` is the X axis, and calling it `X_AXIS_UNIT_VECTOR` makes the line worse. That reasoning is stronger in the renderer than anywhere else, and weaker in something like a settings parser, where an unexplained `4096` probably does deserve a name. A module re-enabling them is applying the same judgment to different material rather than breaking a rule.

## Adapting it

Two things are expected to differ between modules, and neither needs a change here. Checks subtracted because a module's material makes them noisy, which is the common case and where invariant 5 applies: the removal carries its reason in the file. And `CheckOptions` beyond naming, tuned to a module's idioms.

Three things are not adapted. The naming `CheckOptions`, because they encode [Naming](../naming/) and `INTERFACES.md`, which are invariants. The fact that `WarningsAsErrors` is `*` minus explicit exceptions rather than an opt-in list: a module wanting a check to be non-fatal names it, so that the exception is visible. And the header filter, which used to be on this list and is not any more.

## Which headers get analyzed

Both baselines pair `HeaderFilterRegex` with `ExcludeHeaderFilterRegex`, and both patterns are identical across every repository on the same baseline.

```yaml
HeaderFilterRegex: '(include|src|tests)/.*\.(h|hpp|tpp)$'
ExcludeHeaderFilterRegex: '(/vcpkg_installed/|/build/|/liara-interfaces/)'
```

Two things about that are worth knowing, because both were learned the hard way.

**clang-tidy matches these against a header's absolute path.** A pattern anchored with `^` on a repository-relative path — `'^(src|include)/...'` — matches nothing, and the effect is not an error but silence: every header in the repository goes unanalysed while the run reports success. That is what every `.clang-tidy` in this project did until v0.1, which is why `#pragma once` never tripped a check despite being in every header.

**A repository analyses its own headers and no one else's.** The exclusion is what enforces it. Without it, a consumer pulling in `liara/result.h` analyses the contract's headers with the *C++* baseline, and produces dozens of findings about a file governed by the C one — the exact inversion of the rule that `INTERFACES.md` wins on the C side. The contract's headers are analysed by the contract's own baseline, in its own CI. The C baseline excludes `\.generated\.` instead, since the only foreign headers it sees are its own generated ones.
