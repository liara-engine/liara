# ADR 0003 — The Host Composes Modules

- **Status**: Accepted
- **Date**: 2026-08-26
- **Deciders**: Antoine

## Context

Liara is a set of modules that version independently and communicate through a C ABI. Something has to decide which modules exist in a given run, create them, and connect them. The obvious candidate is the core: it is the module every other one seems to revolve around, it is created first, and giving it the job requires no new component.

That instinct had already been acted on before this was written down. The core held a reference to the renderer, drove the frame loop, and produced a render packet the renderer consumed. Nothing about that arrangement looked wrong from inside the core.

It is wrong from outside it. A core that creates a renderer is a core that cannot be used without one, cannot be tested without one, and cannot be versioned independently of one — which removes the reason the modules were separated in the first place. The coupling is also invisible in a static build: the symbols resolve at link time and nothing complains, so the drift accumulates silently and is discovered only when someone tries to load a module at runtime, by which point the dependencies are everywhere.

## Decision

The host composes. The launcher — and later the editor — creates each module independently, negotiates ABI versions with each, wires them together, and owns the frame loop.

A module never creates, loads, references, or links against a sibling module. Its only dependency is `liara-interfaces`. Where two modules must exchange data, they exchange it through a type declared in the contract, passed by the host; the renderer receives a render packet, it does not ask the core for one.

## Alternatives Considered

**The core orchestrates.** Fewer moving parts, and the host becomes a thin `main()`. Rejected: it makes `liara-core` depend on every module it orchestrates, which is precisely the dependency graph the multi-repository layout (ADR 0001) exists to avoid. It also makes "core" mean two unrelated things — a set of foundational services, and an application framework — which is how a core becomes  a place things are put rather than a place things belong.

**A dedicated orchestration module**, sitting between the host and the modules. Rejected as premature: it is the core-orchestrates option with an extra name, and it would have to be composed by something anyway. If the launcher and the editor eventually share enough composition logic to justify extracting it, that extraction is a later, better-informed decision.

**A plugin registry** that modules register themselves into at load time. Rejected for now: it solves discovery, which is not yet a problem, at the cost of making the set of loaded modules a runtime property that no build-time check can verify. Revisit when a host needs to load a module it was not compiled to know about.

## Consequences

The host is the only component that knows the module graph, and it grows as modules are added. This is accepted: composition logic has to live somewhere, and concentrating it in one place that is allowed to know everything is better than distributing it among components that then all know a little too much.

Two modules cannot call each other, even when it would be convenient. Data crosses through a type in the contract and through the host, which means adding an interaction between two modules is a change to `liara-interfaces` — deliberately more expensive than adding a function call, because it is a change to the contract everything depends on.

Testing a module in isolation becomes possible and therefore expected. A module with no siblings to stand up is a module whose tests are cheap, which is the practical payoff of the whole arrangement.

The `-runtime` build configuration exists to keep this decision honest. In it, the launcher links against `liara-interfaces` only and resolves modules through `dlopen`/`LoadLibrary`; a module that has acquired a link-time dependency on a sibling fails there and nowhere else. Without that configuration in CI, this ADR would describe an intention rather than a property.

## Revisit If

- A host needs to load modules it was not compiled to know about, which turns composition into discovery and makes the plugin-registry alternative worth reopening.
- The launcher and the editor accumulate enough shared composition logic that duplicating it costs more than extracting it.
- Passing data through the contract becomes the bottleneck for an interaction that is genuinely hot — measured, not suspected.
