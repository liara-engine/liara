# ADR 0009 — A Hand-Written ECS Rather Than an Existing Library

- **Status**: Proposed
- **Date**: 2026-08-26
- **Deciders**: Antoine

> Proposed rather than Accepted: the ECS arrives in v0.2 and nothing has yet been built on it. This records the decision before the investment, which is the only point at which changing my mind is still cheap. It moves to Accepted when v0.2 closes.

## Context

The core needs a way to hold game state. I want an ECS: it separates data from behaviour, it suits data-oriented layouts, and it lets systems be written independently of each other, which fits an architecture where the renderer and a future physics module both operate on the same entities through different components without knowing about each other.

EnTT and flecs exist, are excellent, are faster than what I will write, and would take an afternoon to integrate.

## Decision

The ECS is written by hand, in `liara-core`. The first implementation uses sparse sets: one sparse array indexed by entity ID and one packed array of component data per component type.

## Alternatives Considered

**EnTT.** Header-only, mature, extremely fast, and the default answer for this problem in C++. Rejected, and I want to be honest about the reason, because "it wouldn't have been fast enough" or "it wouldn't have fit the architecture" would both be false. Neither is true. I rejected it because adopting it would remove the part of this project I am here for.

The stated goal of Liara is learning: graphics programming, modern C++, and large-scale architecture. Writing an ECS teaches memory layout design, template metaprogramming and API design in a way that calling one does not. That is the version of this argument that belongs in a design document, and it is accurate as far as it goes.

The fuller version is that I work on things I can see the bottom of. Assembling a project out of libraries whose internals I do not know is, for me, not the same activity as building something — it is a different task that happens to produce a similar artifact, and it is one I lose interest in quickly. On a project with no deadline and no client, sustained interest is the actual constraint, and a technically superior choice that I stop working on is worse than an adequate one I finish. This is a bad criterion for a team and a decisive one for me, so it is recorded here rather than left implicit and rediscovered later as inconsistency.

**flecs.** More featureful than EnTT, with relationships and queries I would not write myself. Rejected for the same reason, more so: the more it does, the less of it I would understand.

**No ECS** — plain object hierarchies, or arrays of structs with hand-written iteration. Simpler and sufficient for the scale Liara will reach before v1.0. Rejected because it does not compose with the module boundaries: the render packet pattern of `ARCHITECTURE.md` §6.3 depends on the core being able to iterate a specific component set cheaply, and an object hierarchy makes that a traversal rather than a scan.

**Archetype storage** (Bevy, Unity DOTS) instead of sparse sets. Better for multi-component queries, which is most of them, and worse in every other way to implement: fragment management, archetype graph transitions, and structural change deferral. Rejected for v0.x as a first implementation, not on principle. If iteration performance becomes the bottleneck, this is where to go, and switching is an internal change since the ECS is not exposed across the ABI.

## Consequences

It will be slower than EnTT, and probably by a lot at first. This is accepted, with a caveat: the ECS is benchmarked in CI from v0.2 onward, so "slower" stays a number I am looking at rather than a thing I assume is fine.

Bugs in it are mine, including the ones EnTT solved years ago and I have not thought of yet. Entity ID recycling with generation counters, iteration stability under structural change, and lifetime of component references are the three I expect to get wrong at least once each.

It is small enough to hold in my head, which is the point. When something behaves oddly, the explanation is in code I wrote, not in a template instantiation four layers into someone else's header.

The API is mine, so it can change to fit what the engine turns out to need instead of what a library author anticipated. The corresponding cost is that there is no upgrade path and no community answering questions about it.

Sparse arrays grow with the maximum entity ID rather than with the entity count, so a workload that creates and destroys many entities pays memory for the high-water mark. Acceptable at v0.x scale, and a real problem at simulation scale.

## Revisit If

- The benchmarks show the ECS is the frame-time bottleneck, and archetype storage is the fix — which is a change of storage strategy under the same API, not a change of this decision.
- I find myself maintaining it instead of building the engine with it. Writing it is the point; keeping it alive at the expense of everything else is not, and if that happens the honest move is to swap in EnTT and say so in a superseding ADR.
