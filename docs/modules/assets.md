---
title: "liara-assets"
description: The only module that reads the file system, what it hands back, and the boundary that is easiest to get wrong.
sidebar:
  label: liara-assets
  order: 6
---

Arrives with v0.3. Turning bytes on disk into data the engine can use, and owning that data's lifetime. It is the only module that reads the file system.

## Contents

**Loading and decoding.** glTF 2.0 for models, the common image formats through stb_image for textures, SPIR-V for shaders, the common audio formats for sounds. Each loader produces a plain description plus a CPU-side buffer.

**Handle allocation and lifetime.** Every asset is referenced by a stable opaque handle, and consumers keep handles rather than pointers into asset storage. That is what makes hot-reload buildable later without touching a single consumer: the handle survives, the bytes behind it are replaced.

**The residency model.** What is loaded, what is resident, what can be evicted, and who holds a reference to what.

## What it does not hold

No GPU upload, which the renderer does when handed CPU-side data and a handle. No playback, which `liara-audio` does when handed decoded PCM. No ECS, because an asset is not an entity.

No asset pipeline either. Offline compilation, packaging and optimization are out of scope for v0.x, which loads raw files and nothing more.

## The boundary that is easiest to get wrong

`liara-assets` decodes and does not consume. It hands a mesh's vertex data to whoever asks and never learns that the renderer uploaded it. It hands decoded PCM to whoever asks and never learns that the audio module played it.

Every transfer goes through the host, exactly as the render packet does. The moment this module calls into the renderer, the boundary is gone, and nothing in the build will complain about it for months.
