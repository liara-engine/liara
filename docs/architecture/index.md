---
title: Architecture
description: What Liara is, what it refuses to be, and where the reasoning behind each structural decision is recorded.
sidebar:
  label: Overview
  order: 0
---

These pages hold the why. [Modules](../modules/) holds the what, meaning the repository list, the responsibilities and the file layout, and [Tooling](../tooling/) and [Contributing](../contributing/) hold the how.

## What the project is

Liara is a 3D game engine written mostly in modern C++, with Vulkan as the reference renderer. It is a personal project whose first purpose is to learn graphics programming, modern C++ and large-scale software architecture by building something non-trivial. Producing an engine that is actually usable for small games is a second objective, and a real one.

It is a deliberate reboot of an earlier monolithic engine of mine. The reboot exists to apply what that project taught rather than to discard it, and the lesson that shaped everything here is that architectural discipline up front is cheaper than architectural archaeology later.

These pages are the contract the project makes with its future self. Every decision here was made for stated reasons, and changing one should mean revisiting those reasons rather than working around them.

## Goals

**Modular at the build level.** Each major subsystem (rendering, platform, assets, audio, physics, the editor) is a separately versioned library that could in principle be replaced by an alternative implementation, including one written in another language. This is not a flexibility feature for anyone using the engine. It is a discipline mechanism that forces separation of concerns from day one.

**Modern hardware and modern toolchains.** Performance is a first-class concern. The reference renderer targets Vulkan 1.3 and assumes a GPU from the last few years, and compilers are required to support C++20.

**Linux and Windows, both first-class.** Development happens on Arch with Hyprland, and the Windows build is validated in CI on every change. macOS, mobile, consoles and web are not supported and will not be without a deliberate expansion of scope.

**Shippable.** By v1.0, a developer should be able to build a small 3D game with the engine and distribute it as a real AUR package on Linux and a portable archive on Windows, without recompiling the engine.

**Forward-compatible past v1.0**, specifically with editor tooling and with large-scale simulation. Neither is supported in v1.0, and the interfaces are designed so that supporting them later needs no breaking change. [Forward-looking decisions](./forward-looking/) lists the five places where that costs something today.

## Non-goals

**Not a general-purpose engine** competing with Unity, Unreal or Godot on feature breadth. Not every rendering technique, not every input device, not every audio format, not every asset type. A feature arrives when a milestone needs it.

**Not backwards-compatible.** No legacy graphics API, no legacy operating system, no legacy compiler. Somebody whose system cannot run a modern Vulkan stack is not the audience, and [ADR 0010](../adr/0010-vulkan-without-abstraction-layer/) is where that position is argued.

**Not a game in itself.** No default character controller, no UI framework beyond developer tools, no networking layer. The engine is the foundation and the game is built on top of it.

**Not feature-complete before being usable.** Each version ships on its own terms, and an incomplete version that runs beats a complete one that does not exist.

## Where decisions are recorded

The canonical statement of a decision lives in these pages. The [architecture decision records](../adr/) hold the context that produced it: what prompted it, what else was considered, what it costs.

They are append-only. A reversed decision is not edited out, and a new record supersedes it and references the original, which produces an honest history rather than a tidy one.
