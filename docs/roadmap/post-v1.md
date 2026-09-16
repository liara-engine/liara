---
title: "Post-v1.0: themes only"
description: Six themes for v1.x and the shape of v2.0, deliberately unscoped until v1.0 ships.
sidebar:
  label: Post-v1.0
  order: 10
---

The roadmap becomes vague here on purpose. What v1.x and v2.x actually contain will be decided when v1.0 ships, with the benefit of having used the engine and found out what is missing.

The themes below are aspirational and unordered, and they will be prioritised and probably reshuffled at v1.0.

## v1.x themes

**The editor.** The largest single effort after v1.0. A scene editor with a viewport, hierarchy, inspector, asset browser, play and pause controls, and scene save and load. That is roughly what Unity shipped as its first editor in 2005, and it is years of work. It is also the headline of the v1.x line, and the interfaces are already designed for it: [render targets](../architecture/render-targets/) and [forward-looking decisions](../architecture/forward-looking/) are what make it possible without an interface break.

**Scripting.** Hot-reloadable game logic. The first host language is C++ through shared libraries reloaded on change, with Lua, AngelScript and others added as plugins to the scripting host module afterwards. The interface is designed for several languages from the start even though only one is implemented first.

**Physics.** Rigid body dynamics and collision detection. Whether it is written outright or wraps Bullet, Jolt or PhysX is deliberately not decided until this milestone, for the reasons in [forward-looking decisions](../architecture/forward-looking/).

**Advanced rendering.** PBR, cascaded shadow mapping, image-based lighting, post-processing, anti-aliasing options. Added incrementally, each as its own minor release.

**Asset pipeline.** A preprocessing step turning source assets into engine-native formats at build time, meaning KTX2 textures and optimised mesh layouts. Faster to load, smaller in memory.

**Animation.** Skeletal animation, morph targets, animation graphs.

**Advanced developer tools.** A timeline profiler with hierarchical CPU and GPU sampling, replacing v0.4's per-stage breakdown, and a scene tree inspector superseding v0.x's ad-hoc panels.
