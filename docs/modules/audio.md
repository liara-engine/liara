---
title: "liara-audio"
description: Playing sound, and why audio is the clearest replaceability case in the project.
sidebar:
  label: liara-audio
  order: 7
---

Arrives with v0.5. Playing sound.

Audio is the clearest replaceability case in the project. The gap between what a hobby project needs and what a shipped game needs is filled, in practice, by swapping the audio backend for FMOD or Wwise. The engine's job is to make that a module swap rather than a rewrite, which is the whole reason this is a repository and not a directory inside the core.

## Contents

**Device and mixing.** Device selection, the mixing graph, voice allocation and lifetime. miniaudio is the backend, and no miniaudio type crosses the boundary.

**Playback.** Playing a decoded sound, looping, stopping, volume, basic bus routing. Spatialisation is deferred, so v0.x is 2D audio.

## What it does not hold

No decoding. The host hands the module PCM that it got from `liara-assets`.

No file access at all.

No ECS. An emitter component lives in the core, and the host turns it into playback calls, in exactly the way it turns renderable components into a render packet.
