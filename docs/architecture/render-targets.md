---
title: Render targets and multi-view rendering
description: The generalisation that costs nothing now and is the only reason an editor is possible later without breaking the interface.
sidebar:
  order: 5
---

A naive renderer draws a frame by rendering the scene into the swapchain and presenting it. That works for a standalone game and falls apart for editor tooling, where the scene is one panel among several and has to end up in a texture the interface can sample.

So the renderer interface is built around abstract render targets and multi-view rendering from v0.1. A target is an opaque handle that may be a swapchain image, in standalone mode, or an offscreen texture, in an editor, a shadow map, a picking buffer or any intermediate pass. A view is a camera plus a viewport plus a set of render parameters, and one frame may render several views into several targets.

The generalization is essentially free. The standalone case becomes "render one view into the swapchain target", which is no more complicated than the naive version. What the interface gains is that it admits the other cases, so building an editor on top of the engine later needs no change to it.

This is the single most important piece of forward-compatibility work in the engine, and the one that would be most expensive to retrofit. Without it, the editor would be impossible without an interface break, and an interface break is a forced migration of every module at once.
