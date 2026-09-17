---
title: The ECS and the render packet
description: Why an ECS, why a hand-written one, and the extract-and-render pattern that keeps the renderer away from it.
sidebar:
  order: 4
---

Game state is managed through an entity-component-system model: entities are opaque identifiers, components are plain data attached to them, and systems are functions operating on the entities matching a component pattern.

## Why an ECS

It suits the rest of the architecture, which is the whole argument. It separates data from behavior, so the layout is independent of the systems consuming it. It admits data-oriented memory layouts, which suit the hardware. And it lets systems be written independently of each other, which is what the module split needs: a rendering system and a physics system operate on the same entity through different components without either knowing the other exists.

It is not the only model that could work here, and it is not chosen for ideological reasons. It is chosen because it composes with everything else.

## A hand-written one

The ECS is written by hand rather than taken from EnTT or flecs. [ADR 0009](../../adr/0009-in-house-ecs/) records that decision in full, including the part of the reasoning that is about me rather than about the code.

What it aims at is narrower than what a library offers. Correct, meaning entities are not confused with each other and a destroyed entity produces no dangling reference. Fast enough for what this engine does. Small enough to hold in my head. And free to change as experience accumulates, without migrating through somebody else's API changes.

The first implementation uses sparse sets: one sparse array indexed by entity ID and one packed array of component data, per component type. That trades memory, since the sparse arrays grow with the maximum entity ID rather than the entity count, for fast iteration and constant-time lookup. Archetype storage is more cache-friendly for multi-component queries and considerably harder to implement, and it is not chosen for v0.x.

From v0.2 the ECS is benchmarked in CI, so a performance change is visible and deliberate rather than noticed a year later.

## The render packet

The renderer does not touch the ECS. The boundary between the core, which owns the data model, and the renderer, which is a swappable module, is too important to expose ECS internals across.

Instead the core extracts, once per tick, a flat list of plain-old-data structures describing what to draw: transforms, mesh references, material references and view information. That list is the render packet, the renderer consumes it and produces pixels, and once consumed it may be discarded. The pattern is sometimes called extract-and-render, and Bevy uses it under the name `ExtractSchedule`.

Two benefits are not obvious from the shape of it.

It **decouples rendering from simulation timing**. A packet is a snapshot, so the renderer can work on it from another thread without fine-grained synchronization, and the simulation can advance the next tick while the previous one is still being drawn.

It **makes the cross-language story work**. A packet is plain data, so it crosses the C boundary without ceremony, and a renderer written in Rust or Zig receives exactly what the C++ one receives.

The cost is that the core builds a packet every tick, iterating the ECS and copying data. In practice that is a few floats per visible entity, and it is dwarfed by the cost of drawing them.

:::caution[The packet in the headers is a placeholder]
`liara_render_packet_t` as it stands today carries a grid width and height, a background color, and an array of drawables that are an x, a y and a packed `0xAARRGGBB` colour. It is what the Phase 0 demo needs and nothing more. The views, lights, meshes and materials described above arrive with v0.2, when the ECS they are extracted from exists.

The struct carries a `struct_version` field against `LIARA_RENDER_PACKET_VERSION`, which is how that growth happens without a major bump.
:::
