---
title: Planned modules
description: The editor and the physics module. What they will hold, and why neither interface is designed yet.
sidebar:
  label: Planned (editor, physics)
  order: 8
---

Two repositories are named, have their namespaces claimed, and will not be created until the v1.x cycle. Both entries exist so that the v0.x interfaces are designed with them in mind, and neither is a commitment to a design.

## `liara-editor`

The application a developer launches to build a game: it hosts the engine, places entities in a scene visually, edits their components through an inspector, and saves and loads scenes. The analogue of the Unity or Unreal editor.

It does not exist in v0.x, and scenes are built in code or loaded from JSON written by hand. That is a real limitation rather than a design position.

What makes it possible to defer is that the v0.x interfaces are designed so introducing it later needs no interface change, which is what the render-target and multi-view decisions in `architecture.md`<!-- TODO link → the render targets and forward-looking decisions sections, once architecture is split --> are for. An editor is a host that renders the scene into a texture and shows it in a panel, so if the renderer can already draw into something other than the swapchain, the editor needs nothing new from the contract.

When it is built, the repository will hold an application owning its own window and swapchain and driving the engine's tick manually, an ImGui interface (scene hierarchy, component inspector, asset browser, viewport, play and pause and step), gizmo rendering built on the renderer's debug primitives, scene serialization, and project management.

## `liara-physics`

Collision detection and rigid body dynamics.

It does not exist in v0.x either, and entities have transforms that nothing moves except direct ECS manipulation.

The interface is deliberately not designed yet, and that is the difference with the editor. The editor's shape is known well enough to design around; physics is not, and the design will be informed by having used the engine to build a game without it. Designing it now would mean guessing which of collision shapes, constraints, queries and determinism actually matter here, and being wrong in a contract everything depends on.

When it arrives it will likely hold collision shapes, rigid body dynamics with constraints and joints, raycasts and queries, and debug visualization through the renderer. Whether it is written outright or wraps Bullet, PhysX or Jolt is a decision for that milestone.
