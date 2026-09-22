---
title: "liara-framework"
description: The convenience layer a C++ game links instead of calling the C ABI by hand, the three rules that bind it, and the boundary that decides whether a feature belongs to it or to the contract.
sidebar:
  label: liara-framework
  order: 5
---

Today it is a directory rather than a repository: `framework/` in the meta repository, a CMake `INTERFACE` library (headers only, nothing compiled) exported as `Liara::Framework`. The repository this page is named after arrives with the extraction, expected in v0.7 to v0.9. What the layer is for does not depend on its address: it holds what a host or a game would otherwise write for itself on top of the C ABI, and [ADR 0013](../../adr/0013-the-framework-layer/) records why it exists at all.

The position is the one libc occupies in a Unix system, where `liara-interfaces` is the syscall table and the modules are the kernel subsystems. A user program links libc so that it does not issue raw syscalls; a Liara game links the framework so that it does not fill dispatch tables, compare packed version words and check the ABI's `liara_result_t` return code after every call by hand.

## The three rules

**It has no ABI of its own, and that is the point.** The framework is compiled into its consumer, so no symbol of its own crosses a runtime boundary, and `INTERFACES.md`, the interface design rules that live in `liara-interfaces`, does not constrain it. `std::` types, templates, exceptions, RAII, overloads and default arguments are all allowed. It versions as source, and two versions of it never meet in one process. That is what the ABI can never be, and it is the entire reason the layer exists.

**It depends on `liara-interfaces` and on nothing else.** Same rule as a module, for the same reason: it has to work under the `-link` presets, where the host names a CMake target per module, and under the `-runtime` presets, where the host resolves every entry point by name.

**It adds no capability, only ergonomics.** Anything reachable through the framework is reachable through the raw ABI, which is what keeps a game written in another language from being a second-class consumer. The corollary is the part that does the work: a framework feature that cannot be written as ABI calls plus local computation is a defect in the ABI rather than a feature of the framework. That is checkable in review, since a function body here has nothing in it but ABI calls and local computation, except for module loading. Module loading reaches the ABI rather than sitting above it: `ModuleLoader.h` opens the shared library and resolves each entry point by name before any ABI call exists to make, so its `dlopen` and `LoadLibrary` calls are the platform-dependent code that gets a caller to the ABI at all, not a function built from it.

Nothing below the framework can see its state, which follows from the third rule. If something below the boundary would need to know a value, that value does not belong up here.

## The boundary rule

> The ABI carries what cannot be reconstructed above it. Anything a caller can build from existing ABI calls plus local computation belongs to the framework.
>
> The one exception: a convenience enters the ABI if and only if its absence would force a consumer that is not written in C++ to rewrite platform-dependent code.

The exception exists because the framework is C++ first. A game written in Rust or Zig has no framework for a long time.

The cases the rule has decided so far:

| Feature                                                    | Where     | Why                                                                                                                                                    |
|------------------------------------------------------------|-----------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| `time_now_ns`                                              | ABI       | Irreducible, it is a system call                                                                                                                       |
| `time_sleep_until_ns`                                      | ABI       | Irreducible, and the POSIX absolute form cannot be rebuilt from above                                                                                  |
| `sleep_ns(d)`                                              | Framework | `sleep_until_ns(time_now_ns() + d)`, and no platform dependency                                                                                        |
| Game clock: pause, time scale, fixed step                  | Framework | State above the ABI, travelling down as a delta                                                                                                        |
| Human-readable date formatting                             | Framework | Pure computation on `time_wall_ns`                                                                                                                     |
| `time_resolution_ns`, if it ever arrives                   | ABI       | Announced granularity is platform knowledge and is not reliably measurable from above. Held there by the exception clause rather than by the main rule |
| The native window handle struct                            | ABI       | Platform knowledge                                                                                                                                     |
| Physical input to logical action                           | Framework | Declared a consumer's concern by [liara-platform](../platform/) since Phase 0                                                                          |
| RAII handle wrappers, `liara_result_t` turned into a throw | Framework | C++ ergonomics only                                                                                                                                    |

## What it holds today

**Module loading**, in `ModuleLoader.h`. It generates a dispatch table per module from the X-macro list in `liara/<module>/<module>_functions.h`, the file where a namespace declares its entry points as data, one line per function; [ADR 0012](../../adr/0012-generated-module-dispatch-tables/) records why a host generates its table from that list rather than writing it by hand. It opens a shared library with `dlopen` on Linux and `LoadLibrary` on Windows, resolves each entry point by name and names the missing symbol when one is absent, refuses a module whose reported `struct_version` (the layout of the info struct it was compiled against) is older than the one this build understands, and owns the library through an RAII `Module<Api>` under the `-runtime` presets.

**ABI version negotiation**, in `Modules.h`. `ModulesAreCompatible` takes a set of module descriptions and runs the negotiation of [ADR 0005](../../adr/0005-version-encoding-and-compatibility/) on each of them, reporting every incompatible module instead of stopping at the first, so that a mismatched workspace is fixed in one pass.

Both moved out of `launcher/` with no change beyond the namespace. The framework has no test suite of its own yet.

## What it will hold

At least four things are expected:

**The game clock.** Pause, time scale, fixed time step, and human-readable formatting of a wall-clock timestamp. This is the feature that produced [ADR 0013](../../adr/0013-the-framework-layer/), and it arrives around v0.7, when a sample game exists to say what it should do.

**Logical input mapping.** The translation from a physical key, button or axis to a named action, which [liara-platform](../platform/) has refused since Phase 0 and which no repository has held since.

**Handle wrappers.** RAII types over the opaque handles the ABI hands out, and a call-site helper that turns a `liara_result_t` into an exception, so that a game written in C++ is not checking a return code after every call.

**The declared lifecycle.** A game gives the framework its init, update and shutdown, and the framework supplies the `main()` that composes modules and runs the loop. That convention is the part of the delivery question that [ADR 0013](../../adr/0013-the-framework-layer/) settles now; who owns `main()` in a shipped game is answered in v0.7.

## Extraction

`liara-framework` as a repository of its own is expected rather than hypothetical, and the expected moment is v0.7 to v0.9. Two things are true then and are not true now: there is enough material to justify a repository, and the sample game is a second consumer proving the layer serves something other than the launcher. The timing is the third argument, since the move lands before v1.0 freezes the public surface.

Moving it costs nothing in the contract, because the layer has no ABI namespace to rename, which is the split [Modules](../) draws between a repository layout and a namespace. What it costs is a directory move and one more entry in the `MODULES` list of `scripts/liara.py`.
