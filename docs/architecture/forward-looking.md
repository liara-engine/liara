---
title: Forward-looking decisions, and what is deferred
description: The six things designed now for versions that do not exist yet, and the nine deliberately not designed at all.
sidebar:
  order: 9
---

## Designed now

Six decisions are made in anticipation of later versions. Each passed the same test: cheap to do now, expensive to retrofit later. Nothing else in v0.x is designed for a version that does not exist.

**Editor readiness.** The renderer interface uses abstract render targets and admits several views per frame from v0.1, which makes an editor buildable later with no change to it. [Render targets](../render-targets/) covers why this one matters more than any of the others.

**Large-scale readiness.** A transform crossing the boundary uses double precision for translation, and the view structure carries a hint about scene scale, so a KSP-style simulation needs no interface change. [Cross-platform strategy](../cross-platform/) has the reasoning.

**Picking.** The renderer interface will optionally produce an entity ID buffer alongside the color buffer. It stays dormant through v0.x, since nothing requests the buffer, and the interface accepting the option is what stops picking from being a breaking change later.

**Hot-reload readiness.** Assets are referenced by stable handle, and no consumer memoises a raw pointer into asset storage. That single constraint is what makes asset hot-reload implementable later without touching a renderer.

**Subsystem namespaces.** Platform, assets and audio have their own ABI namespaces from the outset, so extracting them into repositories of their own, whenever that happens, is a delivery change rather than an interface break.

**The framework layer's position.** A convenience layer above the C ABI exists from v0.1, and a C++ game is expected to call it rather than the contract, although what it holds today is host machinery and not one game-facing function. The retrofit cost here is paid in the dogfooding phase rather than in the headers: [v0.7 to v0.9](../../roadmap/v0-7-to-v0-9/) finds different frictions depending on whether the sample game is written against the framework or against the raw C ABI, and that phase runs once. What is decided now is where the layer sits and none of its API, which is what [ADR 0013](../../adr/0013-the-framework-layer/) records.

Three of the six describe interface properties that are not in the headers yet, since the transform types, the view structure and the ID buffer all arrive with the milestones that need them. What is decided now is their shape, which is the part that would be expensive to change.

## Not designed at all

Nine concerns are explicitly out of v0.x, and each is revisited only when its milestone arrives.

| Concern                                              | Until                                                  |
|------------------------------------------------------|--------------------------------------------------------|
| Scripting, in any language                           | Post-v1.0                                              |
| Networking                                           | Indefinitely, and out of v1 and v2 scope               |
| Physics                                              | v1.x, and the interface is not designed in v0.x either |
| Skeletal animation                                   | Deferred. Static meshes only in v0.x                   |
| Audio spatialisation                                 | Deferred. 2D audio only in v0.x                        |
| Asset pipeline: compilation, packaging, optimisation | Deferred. Raw loading only in v0.x                     |
| Multi-window                                         | v2.x at the earliest                                   |
| Hot-reload of code, as opposed to assets             | Indefinitely                                           |
| Console and mobile platforms                         | Out of scope                                           |

Refusing to design for these now is not a claim that they do not matter. It is a claim that designing for them now would take speculation more likely to be wrong than right, and would bake that speculation into a contract everything depends on.

Physics is the clearest case, and it is why the list is worth writing down rather than leaving implicit. Its interface is deliberately not designed, because the design will be informed by having used the engine to build a game without it, and guessing now which of collision shapes, constraints, queries and determinism actually matter here would be guessing in exactly the place a wrong guess is most expensive.
