---
title: "ADR 0012: Module entry points as data, and generated dispatch tables"
description: Why each namespace declares its entry points as an X-macro list in the contract, and why a host generates its dispatch table from that rather than resolving symbols by hand.
sidebar:
    label: "0012 · Module dispatch tables"
    order: 12
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-09-20 | Antoine  |

## Context

[ADR 0003](../0003-the-host-composes-modules/) makes the host responsible for creating modules and wiring them together, and the `-runtime` presets exist to keep that claim honest by making the launcher resolve modules with `dlopen` instead of linking them.

What the runtime path actually did until v0.1 was resolve three `liara_<module>_info` symbols, print what they reported, and return before creating anything. It exercised no real call. That was adequate while the launcher did nothing, and it stopped being adequate the moment the launcher started using modules: every function it calls has to be resolved by name in that branch too, or the presets keep passing while testing nothing.

The obvious way to do that is by hand. One `dlsym`, one cast to the right function-pointer type, one null check per function per module, with the symbol name spelled as a string at every site. It scales with functions times modules, the string is duplicated from the declaration it must match, and a function forgotten in the runtime branch is invisible in the link build where the direct symbol still resolves.

Two further requirements shaped the answer.

**A host should not have to think about which preset family it is in.** Somebody adding a call to a module in the launcher should write the call, not discover that it has to be written twice and registered in a third place.

**A module must eventually be implementable in any language with a C FFI.** That is not hypothetical convenience: it is the substitution [Modularity model](../../architecture/modularity/) claims the C boundary buys, and until v0.1 nothing had ever tested it.

## Decision

Each ABI namespace declares its entry points as data, in an X-macro list beside the headers that declare them: `liara/<module>/<module>_functions.h` in `liara-interfaces`.

```c
#define LIARA_CORE_FUNCTIONS(X, prefix)                                              \
    X(prefix, info,   const liara_module_info_t*, (void))                            \
    X(prefix, update, void,                       (liara_core_handle_t*, float))
```

A host generates three things from that one list: a dispatch table whose members are function pointers, a filler that takes link-time addresses, and a filler that resolves by name at run time and refuses, naming the symbol, when one is absent. Call sites are then identical under both preset families.

Four properties follow, and each is a decision in its own right.

**A member's name is the symbol's name with the `liara_<module>_` prefix removed.** The prefix moves to the left of the arrow rather than disappearing, so `liara_core_get_render_packet(...)` becomes `core->get_render_packet(...)`. Members are therefore `snake_case` in C++, against the project's own convention, deliberately: renaming them would introduce a *conversion* between the two worlds, and a conversion is a table a reader has to know.

**The list takes the symbol prefix as an argument**, which is what lets one set of expansion macros serve every module. A macro body cannot contain `#define` or `#undef`, so without it each module would need its own pair of directives per mode.

**A `Module<Api>` object owns the shared library**, and module *instances* remain a separate matter. `create` and `destroy` are ordinary entries a module lists or does not, which is why `liara-platform`, which has neither, needs no special case.

**Every entry is required**, per [ADR 0011](../0011-removing-the-degraded-compatibility-state/).

## Alternatives considered

**A macro at the call site**, hiding the link-versus-runtime choice there. This was the starting idea and it is the one this record replaces. Under the link presets a module call is already an ordinary call — typed, steppable, with a readable error when an argument is wrong — and a call-site macro replaces that with token pasting *in both modes*, paying the cost where there was nothing to hide. It also cannot route `liara_core_update` to the right table without deducing the module from the name, which is preprocessor gymnastics.

**Hand-written resolution per module**, which is what existed. Rejected on the arithmetic: the work grows with functions times modules, the symbol name is duplicated from the declaration it must match, and an omission is invisible in the build most people run.

**A plugin registry**, with modules registering themselves on load. [ADR 0003](../0003-the-host-composes-modules/) already rejected it for composition, and it does not help here anyway: something has to open the library before any registration can run.

**Generating the declarations from the list**, making the list the only source of truth and the double declaration impossible. Genuinely tempting, and it does not break other languages — `bindgen` and Zig's `@cImport` both run the C preprocessor, so generated prototypes reach them intact. Rejected because there is nowhere to attach the per-function Doxygen that v1.0's Definition of Done requires: the comment would have to live inside the macro list, which is worse than the problem.

## Consequences

`liara-interfaces` gains files that exist for a consumer's convenience rather than to declare the ABI. That is a real stretch of what the contract repository is for, accepted because a machine-readable description of a contract belongs with the contract: it is what lets an implementer in another language, the composition tool and a host derive from one source instead of three.

Adding an entry point is one line in the list. Forgetting it is loud three ways: a compile error at the table, a **link** error under the `-runtime` presets if the call bypasses the table, and a load-time refusal naming the symbol if the module does not export it.

The double declaration remains — the documented prototype and the list entry — and nothing verifies that the two agree. Comparing the list against a module's exported symbols in a test would close it, and is the only check that would also work on a module no compiler of ours ever sees. Deferred rather than dismissed.

Table members are `snake_case` in C++, so `readability-identifier-naming` is suppressed at the three expansion sites, with the reason beside it. Three more suppressions cover the X-macro idiom itself, where `cppcoreguidelines-macro-usage` asks for a constexpr template that cannot declare members whose name and signature both vary, and `bugprone-macro-parentheses` asks for parentheses that would make the declarator ill-formed.

The `-runtime` presets stop being the leg that keeps the architecture honest and become the **production path** for any module not written in C or C++, since `link` needs a CMake target per module and a Rust or Zig module has none.

That is no longer an expectation. A Rust `cdylib` standing in for `liara-platform`, including no Liara header and linking against nothing, is loaded and negotiated by an unmodified launcher.

## Revisit if

- A host needs to load a module it was not compiled to know about, which turns composition into discovery and reopens the plugin registry of [ADR 0003](../0003-the-host-composes-modules/).
- The double declaration causes a real incident rather than a theoretical one, which would justify the symbol-comparison test or a generator.
- The lists outgrow what a C macro can express — enough entries to be unreadable, or enough type information that generating default returns becomes possible — which would argue for describing the contract as data and generating the header from it.
- A second host appears and wants a different loading policy, which would mean this belongs to a host rather than to the contract.
