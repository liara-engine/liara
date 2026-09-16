---
title: Cross-platform strategy
description: How Linux and Windows both stay first-class, and the double-precision decision that has to be made before anything depends on it.
sidebar:
  order: 7
---

The engine targets Linux and Windows. Platform-specific code is confined to specific subsystems (windowing, file I/O, and threading primitives where the standard library is not enough) behind portable interfaces, and the bulk of the codebase never knows which platform it is on.

Development happens on Linux, and the Windows build is validated continuously in CI without being the primary target. That makes platform parity an invariant CI enforces rather than a discipline I have to remember, which is the difference between a property and an intention.

There is no runtime platform selector. Each build produces a binary for one platform, with the platform-specific code selected at compile time through preprocessor flags driven by CMake.

## Floating-point precision and world coordinates

One concern spans both platforms and every future version: single-precision floats lose precision a few kilometres from the origin, which confines an engine to small-world games. Anything larger, meaning open world, space or simulation, needs double precision or a floating-origin technique.

The conservative choice is taken. A transform crossing the interface boundary uses double precision for translation, and single precision for rotation as a quaternion and for scale. The renderer converts to single precision when uploading to the GPU, optionally applying a floating-origin transform on the way.

That costs a small amount of memory per entity and is invisible at small scales. It is decided now because changing the transform layout in the interface later would be a major bump of `liara-interfaces` and would invalidate every existing module. Now it costs nothing; later it costs a migration.

The types it applies to do not exist yet. `liara_transform_t` and the rest of the math types arrive in `liara-interfaces` with v0.2, and this is the decision they will be written against.
