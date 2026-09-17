---
title: "liara-interfaces: the contract"
description: The C headers every module implements or consumes, what the repository is forbidden from containing, and why its version rules are stricter than everyone else's.
sidebar:
  label: liara-interfaces
  order: 2
---

The most version-sensitive repository in the project. Every module declares which version of it they need, and a breaking change here ripples through every consumer at once.

## What it is for

It defines the C ABI contracts between modules. Nothing crosses a module boundary without going through a header defined here, and a module using richer C++ types internally wraps them at its public surface so that only the C interface is visible.

## Contents

The public headers, as they stand at the time of writing, are:

```text title="liara-interfaces/include/liara/"
liara/
├── abi_version.h              # the ABI version of the contract itself
├── version.h                  # packing, comparison and compatibility rules
├── result.h                   # the error reporting convention
├── modules.h                  # the self-description entry point every module exports
├── config.h.in                # generated, carries the version at build time
├── internal/
│   └── portability.h          # compatibility macros, C and C++ alike
├── core/
│   ├── core.h                 # the core module's entry points
│   └── core_export.h          # generated visibility macros
└── renderer/
    ├── renderer.h             # the renderer module's entry points
    ├── renderer_export.h      # generated visibility macros
    └── packet.h               # the render packet
```

The namespaces for `platform`, `assets`, `audio` and `physics` are claimed and their directories do not exist yet, which is the schedule of the [overview](./) applied to the contract.

Alongside the headers, the repository carries a `CMakeLists.txt` exposing them as an `INTERFACE` library, a `vcpkg.json` with no runtime dependency and a `tests` feature pulling doctest, the `INTERFACES.md` document specifying how interfaces are designed and evolved, and a test suite that is unlike any other module's.

That suite is the point of the repository. `tests/test_version_c.c` and `tests/test_version_cxx.cpp` check that the headers behave identically under both languages, `tests/zig/test.zig` and `tests/rust/` consume them through a C FFI from outside the C family entirely, `tests/abi/abi_layout.generated.h` freezes struct layouts against a golden copy. The Zig and Rust tests are registered only when the toolchain is present locally, and are mandatory on two CI legs.

## What it does not hold

No implementation: no `.c` or `.cpp` outside the tests, which exercise properties of the headers rather than behavior behind them.

No external dependency of any kind. The headers may use fixed-width integer types from `<stdint.h>` and `<stddef.h>` and nothing else, because the contract has to be includable by a consumer that has never heard of vcpkg.

## Versioning

Strict semantic versioning, with the rules in [What breaks, and what does not](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/breaking-changes/#the-table). A major bump for anything that breaks ABI or source compatibility, a minor bump for purely additive change, a patch bump for documentation and comments.

Two things make that stricter in practice than the sentence above suggests. Below 1.0.0, release-please is configured to bump the minor for a breaking change and the patch for a feature, so the major stays at zero throughout Phase 0. And under the 0.0.x rule of [ADR 0005](../../adr/0005-version-encoding-and-compatibility/), the patch component is significant during that period: a consumer pinned to 0.0.4 does not accept 0.0.5.

The version lives in `version.h` as macros and is checked at load time through `liara_<module>_info()`, which every module exports for exactly this.
