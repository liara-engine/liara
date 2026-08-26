# ADR 0002 — A C ABI as the Inter-Module Contract

- **Status**: Accepted
- **Date**: 2026-08-26
- **Deciders**: Antoine

> Retrospective.

## Context

ADR 0001 put each module in its own repository. That answers where the code lives; it does not answer what modules are allowed to say to each other. Without a second decision, "separate repository" would only mean the same coupling spread across six clones, which would be worse than the monorepo rather than better.

So this decision and ADR 0001 have the same root. Both exist to make a boundary I can't casually step over, and both were chosen because they fit how I work rather than because they are elegant. The repository split makes the boundary visible; the C ABI makes it load-bearing.

The technical constraint on top of that: C++ has no stable ABI. Two C++ libraries built with different compilers, different standard library versions, or different flags cannot reliably pass C++ types between them. Templates, exceptions, RTTI and standard library containers all make it worse. C has a defined ABI everywhere, which is why Vulkan, SDL, libcurl and Lua all expose C interfaces regardless of what they are written in.

## Decision

Every module exposes its functionality through C-linkage declarations in `liara-interfaces`. That is the only contract another module may rely on; the C++ implementation behind it is private.

Across a module boundary: no standard library types in signatures, no templates, no exceptions, no C++ classes. Objects are opaque handles from factory functions, errors are return values, and whoever allocates says who frees.

## Alternatives Considered

**Shared C++ headers between modules.** Much less verbose, and it lets me use the language I am actually writing in. Rejected on the ABI point above: it would rule out ever loading a module built by a different compiler, which kills the runtime-loading path and the "reimplement a module in another language" property along with it. It also would not have created a real boundary — a shared header can expose whatever I feel like exposing that day, and I would have.

**C++ headers with a hand-imposed restriction** — no templates, no standard library, POD only. This gets most of the ABI stability without the C verbosity. Rejected because the restriction is a promise rather than a mechanism. Nothing fails when I break it, and I would break it, probably without noticing.

**A serialization layer or IPC** between modules, one process each. Real isolation, no ABI question at all. Rejected on latency: a frame loop cannot afford a round trip per call. This works well for desktop tooling (Hyprland does it) and not for this.

## Consequences

The interface is verbose, and writing it is not the fun part. Every object needs create/destroy functions, every error is a code the caller has to check, and anything structured needs a POD declaration. The verbosity is paid once per interface and collected by every consumer, but it is paid in advance and the benefit arrives later.

Each module needs a shim translating between the C boundary and its C++ implementation. That shim is where `reinterpret_cast` and exception-to-error-code conversion live, it is easy to get subtly wrong, and it is not covered by the module's own tests as thoroughly as the C++ behind it is.

The contract can be implemented in any language that speaks C, and consumed the same way. The `liara-interfaces` test suite exercises the headers from C, C++, Zig and Rust, which is how I know this is true and not just intended.

`liara-interfaces` becomes the most sensitive repository in the project, since everything depends on it and a breaking change there forces a migration everywhere. That is why it has an ABI pipeline of its own — layout freezing, snapshot diffing, portability and interface rules — and why the version negotiation of ADR 0005 exists at all.

## Revisit If

- The C boundary becomes a measured performance problem in a hot path, rather than a suspected one.
- A module needs to exchange something the C type system genuinely cannot express, and the workaround is worse than the problem.
