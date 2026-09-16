---
title: The interface boundary
description: Why every module boundary is a C interface rather than a C++ one, and what that rules out.
sidebar:
  order: 3
---

Every replaceable module exposes its functionality through a C-linkage interface defined in `liara-interfaces`. That interface is the only contract another module may rely on, and the C++ implementation behind it is private.

## Why C and not C++

C++ has no stable ABI. Two C++ libraries compiled with different compilers, different standard library versions, different optimization levels or different exception handling strategies cannot reliably exchange C++ types across their boundary, and templates, exceptions, RTTI and standard library containers each make it worse.

C has a well-defined ABI on every platform, and two C libraries exchange data regardless of how either was compiled. It is why every serious cross-language ecosystem exposes a C interface whatever it is implemented in: Vulkan, SDL, libcurl and Lua all do.

Five things follow at a module boundary, and none of them is negotiable:

- No standard library types in a signature. No `std::vector`, no `std::string`, no `std::function`.
- No templates crossing. They may exist freely inside a module.
- No exceptions crossing. Errors are returned as values.
- No C++ classes exposed. Objects are opaque handles created and destroyed through factory functions.
- Allocation responsibility stated explicitly. A caller knows whether a function allocates, and which function frees the result.

The interface is verbose, and it is also stable, language-agnostic and debuggable. The verbosity is paid once, by whoever designs the interface. The stability is collected forever, by every consumer. [ADR 0002](../adr/0002-c-abi-as-the-inter-module-contract/) records the alternatives that lost, including the one I would recommend to most other people.

## Versioning it

Every module reports its version through `liara_<module>_info()`, and the host checks that against what it requires before calling anything else. A version is a packed 32-bit word rather than a string, and the compatibility rule that decides whether two of them can work together is one function applied in one direction.

The reasoning is in [ADR 0005](../adr/0005-version-encoding-and-compatibility/), including the divergence between the specification and the code that the rule was written to settle.

## What lives in the contract

Headers, and nothing else. The precise contents, the layout and what the repository is forbidden from holding are in [`liara-interfaces`](../modules/interfaces/), and the rules for designing and evolving an interface are in that repository's own `INTERFACES.md`<!-- TODO link → the interfaces guide, once interfaces is split into a directory -->, which is required reading before touching anything in it.

Those two documents are deliberately the detailed ones, and this page is deliberately not. A rule stated in two places is a rule that will eventually be stated differently in each.
