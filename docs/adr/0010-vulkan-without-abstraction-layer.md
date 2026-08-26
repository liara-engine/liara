# ADR 0010 — Vulkan Directly, Without a Rendering Abstraction Layer

- **Status**: Proposed
- **Date**: 2026-08-26
- **Deciders**: Antoine

> Proposed rather than Accepted: real Vulkan work starts in v0.1 and only a placeholder renderer exists today. Moves to Accepted when v0.1 closes.

## Context

This is the third engine project I have started, and the graphics API question has been answered three times.

The first attempt, years ago, was on Windows, and I chose DirectX — partly following Arash Khatami's Game Engine Series, mostly because on Windows it was the obvious choice. Cross-platform support was not a consideration. That project never got past a sketch.

The second attempt came after I moved to Arch and discovered an operating system could adapt to me rather than the reverse. I wanted something that ran on Windows and Linux, was modern and actively maintained, performed well, and was actually used professionally. Vulkan met all four better than anything else. That project — Liara's predecessor — went a long way on it.

Liara is a reboot of that project, undertaken because the architecture had stopped fitting what I wanted, not because the rendering choice had. I had enjoyed Vulkan and had no reason to reopen the question, so I did not.

One thing carried across all three attempts: I have little patience for backwards compatibility and legacy support. I find it usually costs more than it returns. It is why the engine targets Vulkan 1.3 and recent GPUs, and why I develop on Arch rather than Debian.

## Decision

Vulkan 1.3 directly, through Vulkan-Hpp and VMA, as the reference renderer. No abstraction layer over the graphics API and no second backend. A user whose hardware cannot run a modern Vulkan stack is not the target audience.

## Alternatives Considered

**A rendering abstraction layer** — bgfx, sokol_gfx, wgpu — with Vulkan as one backend among several. I should be straightforward here: these were not weighed at the time. I did not know they existed when the choice was made, and I only learned about them while writing this record.

Knowing about them now does not change the answer, for a reason worth stating plainly rather than reconstructing as an evaluation I never performed. I build things to understand them from the bottom, and an abstraction layer removes precisely the layer I want to be working in. It would let me draw a triangle in a fraction of the time and teach me nothing about how a triangle gets drawn. Since learning graphics programming is the first of the three reasons this project exists, that is disqualifying regardless of the library's quality.

The honest framing: for a project whose goal is to ship a renderer, an abstraction layer is very likely the correct choice. For this project, it defeats the purpose. That is a statement about the project, not about the libraries.

**OpenGL**, or a Vulkan renderer with an OpenGL fallback for older hardware. Rejected on the legacy point above. Supporting old hardware means two rendering paths, two sets of bugs, and design decisions constrained by the weaker one. `ARCHITECTURE.md` §2.2 already declares broad hardware support a non-goal; this is that non-goal applied.

**DirectX 12**, as in the first attempt. Rejected on portability: Linux is the primary development platform now, and a Windows-only renderer would make the platform I actually use the secondary one.

**Deferring the choice** behind the module's C interface, and deciding later. Partially real — `liara-renderer` is a replaceable module, and someone could implement the same interface over another API without touching the rest of the engine. But the interface has to be designed against something, and designing it against Vulkan's model (explicit synchronisation, command buffers, descriptor sets) is a choice that shapes it whether or not it is acknowledged. Better to say so.

## Consequences

Everything takes longer. Vulkan requires explicit synchronisation, memory management, descriptor handling and swapchain recreation before it draws anything, so the first triangle is weeks of work rather than an afternoon. This is the accepted cost and, for this project, part of the point — but it does mean v0.1 is a large milestone by any measure other than what it displays.

There is a real risk of stalling in it. This is what happened to the previous project, in the architecture rather than in the rendering, and the mechanism is the same: a task that is harder than it should be is a task that does not get done. Vulkan is legitimately hard, so the milestone structure has to keep producing something visible rather than accumulating infrastructure. That is what `ARCHITECTURE.md` §3.5 is for.

Vulkan-Hpp and VMA are themselves abstraction layers, which would sit oddly next to the reasoning above if I had not already done without them. The predecessor project — also called Liara; changing the name would have been more practical, but I like it and how it sounds — was written against raw Vulkan, following Brendan Galea's series at the start. I have written the allocator, the descriptor handling and the boilerplate by hand, badly, and I know what these libraries are doing on my behalf.

That is the rule I apply, and it is narrower than "no abstractions": I allow a library that does better something I have already done myself and still understand. Vulkan-Hpp is the same API with C++ types over a model I know. VMA solves an allocation problem I have solved worse. The same test is why I use the standard library without hesitation — over the years I have reimplemented a fair amount of what it does, sometimes for coursework, sometimes because I did not know it was already there — and why bgfx fails: it would be the first time I used something whose model I had never built.

Hardware support is narrow and stays narrow. No integrated GPU from before Vulkan 1.3, no fallback, no software path. Accepted.

The renderer is the largest single module and the one most likely to need rewriting as I learn. The C boundary at least contains that: a rewrite behind the interface does not propagate.

## Revisit If

- Hardware support becomes an actual barrier to someone shipping a game with the engine — which is a v0.6 or later question, not a v0.1 one.
- A second backend becomes necessary for a platform worth supporting, at which point the abstraction question genuinely reopens and should be answered with a new ADR rather than by retrofitting one onto the interface.
