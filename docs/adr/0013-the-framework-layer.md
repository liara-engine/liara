---
title: "ADR 0013: A framework layer above the C ABI"
description: Why a convenience layer sits between a game and the C ABI, the three rules that bind it, and why the question of who owns main() is left to v0.7.
sidebar:
    label: "0013 · The framework layer"
    order: 13
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-09-21 | Antoine  |

## Context

The platform time design produced two sleep entry points, `time_sleep_ns(duration)` and `time_sleep_until_ns(deadline)`, and the second reconstructs the first in one line: `sleep_until_ns(time_now_ns() + duration)`. Both are useful to a caller, one of the two is redundant in the contract, and nothing written down said which of them belonged in the C ABI.

The missing rule was a missing layer. Four facts say so, and every one of them was already true before the time design started.

**The taxonomy has four kinds of repository and a linked library fits none of them.** [Modules](../../modules/) names the contract, the modules, the hosts and the infrastructure. A library linked into its consumer exports C++ symbols, carries no ABI namespace and no `liara_<name>_info()` entry point, so it fails the substitution test of [ADR 0004](../0004-module-boundaries/) and is not a module: it has no C interface, it is what consumes one. It is not a host either, by that page's own test for one: a host exports nothing and nobody links against it, and being linked is the whole of what a library is for.

**The roadmap promised the layer without defining it.** The Definition of Done of [v0.7 to v0.9](../../roadmap/v0-7-to-v0-9/) reads "A small 3D game has been built using only the public engine APIs", and until this record the only public API of the project was the raw C ABI, so the dominant friction it was scheduled to find was that writing a game directly against a C ABI is unpleasant. No change to the ABI fixes that, because the ABI is austere on purpose: no `std::` types, no templates, no exceptions, opaque handles from `create` and `destroy` pairs. What the dogfooding phase finds depends on which layer the sample game is written against, which is why the layer is decided in v0.1 rather than in v0.7.

**A second feature had already been sent to a layer that did not exist.** [liara-platform](../../modules/platform/) says that "Mapping a physical input to a logical action is a consumer's concern", and lists "No logical input mapping" under what the module does not hold. That concern has had no address since Phase 0. Time is not the first case, only the first one noticed.

**ADR 0003 left a neighboring door open.** Its "Revisit if" list carries "The launcher and the editor accumulate enough shared composition logic that duplicating it costs more than extracting it", and the alternative it rejected in that area was a dedicated orchestration *module*, composed by something else in turn. A library is not a module, so this record does not reopen [ADR 0003](../0003-the-host-composes-modules/). It gives the shared composition logic an address before `liara-editor` exists to duplicate it.

## Decision

A layer called `Liara::Framework` sits between a game and the C ABI, and a C++ game is expected to call it rather than calling the contract directly. Five parts, of which only the last is revisable without a new record.

**The taxonomy gains a fifth kind, the library.** It is linked into its consumer, it exports C++ symbols, it carries no ABI namespace and no `info()` entry point, and it is not substitutable across a C boundary. [Modules](../../modules/#the-libraries) carries the row and the test that separates it from the two kinds beside it.

**Three rules bind it.** It has no ABI of its own, so the constraints of `INTERFACES.md` (the interface design rules that govern the C side) do not reach it, and `std::` types, templates, exceptions, RAII and overloads are all allowed. It depends on `liara-interfaces` and on nothing else, which is the same rule a module obeys and for the same reason: it has to compile and run under the `-link` presets, where a host names a CMake target per module, and under the `-runtime` presets, where a host resolves every entry point by name. And it adds no capability, only ergonomics, so anything reachable through it is reachable through the raw ABI. [liara-framework](../../modules/framework/#the-three-rules) has the reasoning for each.

**The boundary between the two is decided by a rule with one named exception, stated in [liara-framework](../../modules/framework/#the-boundary-rule).** It divides on reconstructibility, and carries one exception for a consumer that is not written in C++. The worked cases are on that page, including the one that takes `time_sleep_ns` out of the ABI and makes `sleep_ns` the framework's first function.

**A game declares its lifecycle to the framework (init, update, shutdown) instead of writing its own loop.** That is the whole of what is settled about delivery. Once it holds, who owns `main()` stops being an architecture decision: standalone, the framework supplies the `main()` that composes modules and runs the loop; hosted, the same three functions are exposed to a runtime that drives them. SDL3 does exactly this with `SDL_AppInit` and `SDL_AppIterate` under `SDL_MAIN_USE_CALLBACKS`.

**It lives in `framework/` in the meta repository, created now.** [liara (meta)](../../modules/meta/) is the repository for everything that has to know about every module at once, and a layer that composes modules is that by definition. The thing that is expensive to change in this project is an ABI namespace, and this layer has none, so its location is a delivery decision and stays revisable.

## Alternatives considered

Three, all defensible, and the first is kept open rather than closed.

**The runtime owns `main()` and opens the game as a shared library**, through a `liara_game_*` ABI namespace, which is the shape Unity and Unreal present to a game author. The objection that it subjects the game to the C ABI holds only for the inversion points, meaning the three or four functions the runtime calls into the game; every other call a game makes goes downward through the framework in idiomatic C++, the way a kernel module exports `module_init` and `module_exit` and then calls whatever kernel API it wants. The machinery is already built, since it is what the module loader does. Rejected as premature rather than as wrong: deciding it now would freeze the delivery shape before a line of game code has been written, and would put `liara_game_*` on the permanent maintenance list, where a rename is a major bump of `liara-interfaces`.

**A library with no inversion ever possible**, where the game owns `main()`, writes its own loop, and the framework is only a set of helpers it calls. Simpler, and it needs no lifecycle convention at all. Rejected because the editor of v1.x is expected to run a game inside a panel, and a game that owns its loop cannot be driven by a host that owns another one. The declared lifecycle costs a game three functions instead of a `while` and is what keeps that option reachable without a breaking change.

**The pragmatic boundary**, where the ABI keeps whatever makes it comfortable to use on its own and the framework takes the rest. Rejected on its criterion rather than on its results: "a convenience every consumer would rewrite anyway" cannot be checked by anyone other than the person proposing the function, so it decides each case by taste and decides the same case differently a year apart. That vagueness is what produced two sleep entry points, and removing it is what this record is for.

## Consequences

The taxonomy decides five ways instead of four, so whatever is built next answers one more question before it gets a repository or a directory. The fifth row, and the count in the page's own description, are in [Modules](../../modules/).

The meta repository releases a fourth release-please package, `framework`, on `framework-vX.Y.Z` tags, seeded at `0.1.0`. Its `CMakeLists.txt` reads no version today, since a CMake `INTERFACE` library compiles nothing of its own and so has no generated header to report one in.

A game written in Rust or Zig has no framework and calls the C ABI directly, for as long as no second framework exists. What it gets instead is the exception clause, which is the only reason a convenience ever enters the ABI, and the cross-language tests that already load a Rust shared library as a module are what keep that path exercised.

The framework becomes a second place where engine behavior could accumulate, which is the failure mode this layer invites. The third rule is the guard, checkable in review rather than by judgment, with the platform-dependent code inside module loading as its one named exception; [liara-framework](../../modules/framework/#the-three-rules) states the check and the exception in full.

## Revisit if

- There is enough material in `framework/` and a second consumer using it, which is expected in v0.7 to v0.9 when the sample game arrives, and which is the moment to extract it into `liara-framework` before v1.0 freezes the public surface.
- The sample game of v0.7 says which delivery shape it wants, which is when the `liara_game_*` alternative is decided rather than deferred again.
- A framework feature cannot be written as ABI calls plus local computation. That is a defect in the ABI rather than a feature of the framework, and it means the boundary rule put something on the wrong side of the line.
