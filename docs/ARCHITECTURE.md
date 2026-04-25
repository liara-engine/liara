# Architecture

> Foundational design document for the Liara Engine project.
> This document captures the **why** behind the engine's structure.
> For the **what** (concrete module list, responsibilities, file layout),
> see [`MODULES.md`](MODULES.md). For the **how** (build, CI, contribution
> workflow), see [`TOOLING.md`](TOOLING.md) and [`CONTRIBUTING.md`](CONTRIBUTING.md).

---

## 1. Project Identity

Liara is a 3D game engine written primarily in modern C++ with Vulkan as the
reference renderer. It is a personal project whose primary purpose is to learn
graphics programming, modern C++, and large-scale software architecture by
building something non-trivial. Producing an engine that is actually usable for
small games is a secondary, but real, objective.

The project is a deliberate reboot of an earlier monolithic engine. The reboot
exists not to discard the earlier work, but to apply lessons learned from it,
the most important of which is that **architectural discipline upfront is
cheaper than architectural archaeology later**.

This document is the contract that the project makes with its future self.
Every decision recorded here was made for stated reasons, and changing a
decision should require revisiting those reasons.

---

## 2. Goals and Non-Goals

The architecture is shaped by what the project tries to be, and equally by
what it explicitly refuses to be.

### Goals

The engine aims to be **modular at the build level**, meaning that each major
subsystem (rendering, editor, physics, scripting host) is a separately
versioned library that can in principle be replaced by an alternative
implementation, including one written in another language. This is not a
flexibility feature for end users; it is a discipline mechanism that forces
clean separation of concerns from day one.

The engine targets **modern hardware and modern toolchains**. Performance is
a first-class concern. The reference renderer uses Vulkan 1.3 and assumes
GPUs that have shipped within the last several years. Compilers are required
to support C++20; older standards are explicitly not supported.

The engine targets **Linux and Windows as first-class platforms**. Development
happens on Linux (Arch with Hyprland), and the Windows build is validated in
CI on every change. macOS, mobile platforms, consoles, and web are not
supported and will not be supported without a deliberate scope expansion.

The engine aims to be **shippable**. By version 1.0, a developer should be
able to build a small 3D game with the engine and distribute it as a real
package on the AUR for Linux and as a portable archive for Windows, without
recompiling the engine itself.

The engine aims to be **forward-compatible** with use cases beyond v1.0,
specifically with editor tooling (Unity/Unreal-style) and large-scale
simulation (KSP-style). These use cases are not supported in v1.0, but the
core interfaces are designed so that supporting them later does not require
breaking changes.

### Non-Goals

The engine does **not** aim to be a general-purpose engine that competes with
Unity, Unreal, or Godot on feature breadth. It does not aim to support every
rendering technique, every input device, every audio format, or every asset
type. Features are added when they are needed for a defined milestone, and
not before.

The engine does **not** aim to maximize backwards compatibility. It does not
support legacy graphics APIs, legacy operating systems, or legacy compilers.
A user whose system cannot run a modern Vulkan stack is not the target
audience.

The engine does **not** aim to provide a complete game in itself. There is no
default character controller, no default UI framework beyond developer tools,
no default networking layer. The engine provides the foundation; the game is
built on top.

The engine does **not** aim to be feature-complete before being usable. The
project follows an iterative model where each version is shippable on its own
terms, and where an incomplete version is preferred to a complete vaporware.

---

## 3. Core Principles

The principles below are the lens through which every architectural decision
is evaluated. When a tradeoff arises, the principle wins by default; when a
principle is overridden, the override is documented.

### KISS as a Cognitive Discipline

The simplest design that meets the stated requirements is preferred. This is
not an aesthetic preference; it is a survival mechanism for a solo developer
who cannot afford to drown in accidental complexity. Whenever a design
decision can be made smaller, less coupled, or more local, it should be.

### Modular by Construction, Not by Convention

A module is not "modular" because it lives in a separate folder. A module is
modular when removing it, replacing it, or recompiling it independently is
mechanically possible. Every claim of modularity is tested by asking: could
someone reimplement this in another language and link it in instead? If the
answer is no, the module is not modular.

### Interfaces Before Implementation

The boundary between modules is more important than the inside of any module.
Interfaces are designed first, frozen carefully, and changed reluctantly.
Implementation can iterate freely; interfaces cannot.

### Performance is a Design Choice, Not an Optimization Pass

Memory layout, allocation patterns, cache behavior, and threading model are
considered at design time. The engine does not write naive code with the
intent of profiling later; it writes code that has thought about performance
upfront, and profiles to validate.

### Visible Progress at Every Step

No version of the engine is allowed to be "internal refactoring with nothing
to show". Every release produces a demonstrable artifact: a binary that runs,
a test that passes, a piece of documentation that explains something new.
This protects motivation and makes regressions obvious.

### The Future Reader is the Author

The author six months from now is a different person than the author today,
and the project must be navigable by that future person without context held
only in memory. This means decisions are documented at the time they are made,
not reconstructed later. It also means consistency across the project matters
more than local cleverness.

---

## 4. Modularity Model

The engine is structured as a **collection of separately versioned libraries
linked together at build time**. There is no plugin loader, no dynamic
discovery, no configuration file that lists which modules to load. Module
selection is made by the build system, frozen at compile time, and does not
change at runtime.

### Why Build-Time Selection

Three models were considered for module composition. The first, used by
projects like Bevy, selects modules at build time via build system options;
substituting a module requires recompilation. The second, used by projects
like Unreal, loads dynamic libraries at runtime according to a manifest;
substitution does not require recompilation but introduces an ABI versioning
problem at runtime. The third, used by projects like Hyprland, runs each
component as a separate process communicating over IPC; this is excellent
for desktop tooling but unworkable for the latency requirements of a game
loop.

The first model was selected because it provides the modularity benefits the
project actually cares about (clean interfaces, replaceability in principle,
forced discipline) without the operational complexity of runtime plugin
loading. Distribution is also drastically simpler: a Liara-based game ships
as a single executable plus its assets, with no plugin discovery mechanism
to debug.

### Designed for Dynamic, Implemented as Static

Although modules are linked statically today, the **interfaces between them
are designed as if they were dynamic**. This means the boundary between any
two modules uses C linkage, plain-old-data types, opaque handles, and
explicit version negotiation. The cost of this discipline is small at the
interface boundary; the benefit is that switching to runtime loading later
becomes a runtime concern (writing a loader) and not an architectural rewrite.

This decision also has an immediate benefit independent of any future
dynamic loading: it makes it possible, in principle, to replace any module
with an implementation written in another language. A user who wanted to
write the renderer in Rust against OpenGL, or the physics module in Zig,
could do so by implementing the same C interface. Whether this ever happens
in practice is a separate question; what matters is that nothing in the
architecture forbids it.

### Multi-Repository Layout

The codebase is split across multiple Git repositories, one per module, all
hosted under the `liara-engine` GitHub organization. This is a deliberate
choice that prioritizes clean separation over operational convenience.

The tradeoff is explicit. A monorepo would be easier to refactor across,
easier to bootstrap for new contributors, and easier to keep version-locked.
A multi-repo layout requires coordination across pull requests, a more
elaborate workspace setup for local development, and an explicit
compatibility matrix to track which versions of which modules work together.

The multi-repo layout is preferred because the cognitive cost of mixing
unrelated concerns in a single repository is, for this developer, higher
than the cognitive cost of running a workspace bootstrap script. The
discipline imposed by separate repositories is treated as a feature, not
overhead.

The full list of repositories and their roles is documented in
[`MODULES.md`](MODULES.md).

---

## 5. The Interface Boundary

Every replaceable module exposes its functionality through a C-linkage
interface defined in the `liara-interfaces` repository. This interface is
the **only contract** that other modules may rely on; the C++ implementation
behind it is private to that module.

### Why C and Not C++

C++ has no stable ABI. Two C++ libraries compiled with different compilers,
different standard library versions, different optimization levels, or
different exception handling strategies cannot reliably exchange C++ types
across their boundary. Templates, exceptions, RTTI, and standard library
containers all complicate this further.

C, by contrast, has a well-defined ABI on every platform. Two C libraries
can exchange data regardless of how they were compiled. This is why every
serious cross-language ecosystem (Vulkan, libcurl, SDL, Lua) exposes a C
interface even when its implementation is in another language.

A C interface across module boundaries means:
- No standard library types in function signatures (no `std::vector`, no
  `std::string`, no `std::function`).
- No templates exposed across boundaries; templates may exist inside a
  module's implementation.
- No exceptions across boundaries; errors are returned as values.
- No C++ classes exposed; objects are opaque handles created and destroyed
  through factory functions.
- Allocation responsibility is explicit: the caller of a function knows
  whether the function allocates, and if so, which function frees the
  result.

The C interface is verbose. It is also stable, language-agnostic, and
debuggable. The verbosity is paid once, in the interface design; the
stability is collected forever, in every module that consumes the interface.

### Versioning the Interface

Each interface header declares its version using preprocessor macros. The
versioning scheme follows Vulkan's pattern: a single 32-bit integer encodes
major, minor, and patch components, and each module exports a function that
reports the interface version it implements.

When the core loads a module (whether statically at link time or dynamically
in the future), it checks that the module's interface version is compatible
with what the core expects. A major version mismatch is a hard error; a
minor version mismatch is a warning; a patch version mismatch is silent.

The semantics of the version components are:
- A **major** bump is required for any breaking change: removed functions,
  changed function signatures, changed struct layouts, changed enum values.
- A **minor** bump is required for additive changes: new functions, new
  enum values added at the end, new optional fields in versioned structs.
- A **patch** bump is for documentation, comments, and other changes that
  cannot affect compiled code.

The detailed rules and the precise list of what counts as a breaking change
are documented in [`INTERFACES.md`](../liara-interfaces/INTERFACES.md) within
the `liara-interfaces` repository.

### What Lives in `liara-interfaces`

The `liara-interfaces` repository contains nothing but C headers. It does
not contain implementation, tests, or platform-specific code. It is consumed
as a header-only library by every other module.

This separation matters because `liara-interfaces` is the most
version-sensitive part of the project. Every other module declares which
version of `liara-interfaces` it requires, and the compatibility matrix in
the meta repository tracks which combinations are known to work.

---

## 6. Entity-Component-System Model

The engine uses an Entity-Component-System (ECS) approach for managing game
state. Entities are opaque identifiers, components are plain data attached
to entities, and systems are functions that operate on entities matching
specific component patterns.

### Why ECS

ECS suits the engine's goals for several reasons. It separates data from
behavior, which makes the data layout independent of the systems that
consume it. It enables data-oriented memory layouts, which suit modern
hardware. It allows systems to be authored independently of each other,
which suits the modular architecture: a renderer system and a physics
system can both operate on the same entity through different components,
without coupling.

ECS is not the only approach that could work, and it is not chosen for
ideological reasons. It is chosen because it composes naturally with the
rest of the architecture.

### A Hand-Written ECS

The engine uses a hand-written ECS implementation rather than an existing
library (EnTT, flecs, etc.). This is a deliberate choice grounded in the
project's primary goal: learning. Implementing an ECS from scratch teaches
template metaprogramming, memory layout design, and API design in a way that
adopting a library does not.

The hand-written ECS does not aim to compete with EnTT on performance or
feature completeness. It aims to be:
- Correct (entities are not confused with each other; deleted entities do
  not produce dangling references).
- Sufficiently fast for the engine's use cases.
- Small and comprehensible (a single developer can hold the entire
  implementation in their head).
- Iterable: improvements can be made over time as experience accumulates,
  without having to migrate through a library's API changes.

The initial implementation uses a sparse-set storage strategy: each
component type has a sparse array indexed by entity ID and a packed array
of component data. This trades memory (the sparse arrays grow with the
maximum entity ID) for fast iteration and constant-time lookup.
Archetype-based storage (à la Bevy/Unity DOTS) is more cache-friendly for
queries with multiple components but is significantly more complex to
implement; it is not chosen for v0.x.

The ECS is benchmarked in CI from v0.2 onward, so that performance changes
are visible and intentional.

### The Render Packet Pattern

The renderer does **not** access the ECS directly. The boundary between
core (which owns the ECS) and renderer (which is a swappable module) is too
important to expose ECS internals across.

Instead, each frame, the core extracts a flat list of plain-old-data
structures from the ECS and passes this list to the renderer. This list is
the **render packet**: a snapshot of "what to draw this frame", containing
transforms, mesh references, material references, and view information.
The renderer consumes the render packet and produces pixels.

This pattern is sometimes called "extract and render" and is used by Bevy
(its `ExtractSchedule`) and conceptually by Unity DOTS. It has two
non-obvious benefits.

First, it **decouples rendering from simulation timing**. The render packet
is a snapshot, so the renderer can process it on a separate thread without
fine-grained synchronization with the simulation. The simulation can advance
the next frame while the renderer is still drawing the previous one.

Second, it **simplifies the cross-language story**. Render packets are POD;
they cross the C interface boundary cleanly. A renderer in Rust or Zig
receives the same render packet that the C++ renderer receives.

The cost of this pattern is that the core must build the render packet each
frame, which involves iterating over the ECS and copying data. In practice
this cost is small (the data being copied is just a few floats per visible
entity) and is dwarfed by the cost of actual rendering.

---

## 7. Render Targets and Multi-View Rendering

A naive renderer "draws a frame" by rendering the scene into the swapchain
and presenting it. This works for a standalone game but breaks down for
editor tooling, where the scene is one of many UI panels and must be
rendered into a texture that the UI can sample.

Liara's renderer interface is designed around **abstract render targets and
multi-view rendering** from v0.1. A render target is an opaque handle that
may correspond to a swapchain image (in standalone game mode) or to an
offscreen texture (in editor mode, or for shadow maps, picking buffers, or
other intermediate passes). A view is a camera plus a viewport plus a set
of render parameters; a frame may render multiple views to multiple targets.

This generalization is essentially free. The standalone game case becomes
"render one view to the swapchain target", which is no more complex than
the naive approach. But because the interface admits multiple views and
arbitrary targets, building an editor on top of the engine later does not
require modifying the renderer interface.

This decision is the single most important piece of forward-compatibility
work in the engine. Without it, the future editor would be impossible
without an interface break.

---

## 8. Tick Model and Application Loop

The core does not own the application loop. Instead, it exposes a **manual
tick** function that advances the simulation by a given delta time. The
caller of the core is responsible for the loop itself.

In standalone game mode, the loop is implemented by a small launcher in the
meta repository: poll input, call `liara_core_step(dt)`, build render
packet, render, present, repeat. In editor mode (post-v1.0), the loop is
owned by the editor, which calls the core's tick function only when the
user has pressed Play, and otherwise updates only the editor UI.

This separation matters because the editor and the standalone runtime have
genuinely different loop structures. Forcing them to share a loop owned by
the core would either constrain the editor unnecessarily or pollute the
core with editor-aware branching. Inverting the relationship — making the
loop external — keeps the core focused on simulation and lets each consumer
build the loop that suits it.

---

## 9. Cross-Platform Strategy

The engine targets Linux and Windows. Platform-specific code is isolated in
specific subsystems (windowing, file I/O, threading primitives where the
standard library is insufficient) and is hidden behind portable interfaces.
The bulk of the codebase is platform-agnostic.

Development happens on Linux. The Windows build is validated continuously
in CI but is not the primary development target. This means platform parity
is a CI-enforced invariant rather than a manual discipline.

The engine does not abstract over platforms via a runtime selector. Each
build produces a binary for one platform, with platform-specific code
selected at compile time via preprocessor flags driven by CMake.

### Floating Point Precision and Coordinate Spaces

A specific cross-platform and cross-version concern is **floating point
precision for world coordinates**. Single-precision floats become
imprecise at distances above a few kilometers from the origin, which
limits the engine to small-world games. Larger-scale games (open world,
space, simulation) require either double precision or floating-origin
techniques.

The engine adopts the conservative choice: **transforms exposed across the
interface boundary use double precision for translation**, single precision
for rotation (as a quaternion) and scale. This costs a small amount of
memory per entity and is invisible at small scales. The renderer converts
to single precision when uploading to the GPU, optionally applying a
floating-origin transform.

This decision is made now because changing the transform layout in the
interface later would be a major-version-breaking change to
`liara-interfaces`. Making it now costs nothing; making it later would
invalidate every existing module.

This decision is one of two pieces of forward-compatibility work for the
KSP-style use case (the other being the render target / multi-view design
covered in section 7). No further KSP-specific work is done in v0.x.

---

## 10. Performance Philosophy

The engine treats performance as a property of design, not as a property
of optimization passes. The principle is not "make it work, then make it
fast"; the principle is "design something whose data flow does not
prohibit being fast, and verify that it is fast enough by measuring".

Concretely:
- Hot paths are identified at design time, not discovered in a profiler.
- Allocation patterns are explicit. Code that runs every frame should not
  allocate; code that runs at startup may allocate freely.
- Data layout follows access patterns, not object-oriented intuition.
- Benchmarks are written alongside features, not retrofitted.

The engine does not pursue micro-optimizations that compromise readability.
Cache-friendliness, allocation discipline, and avoiding unnecessary work
account for the vast majority of the gains; SIMD intrinsics and assembly
hand-tuning do not appear in v0.x.

---

## 11. Forward-Looking Decisions

A small number of architectural decisions are made now in anticipation of
later versions. Each has been validated as cheap-to-do-now and
expensive-to-retrofit-later.

**Editor readiness** — The renderer interface uses abstract render targets
and supports multiple views per frame from v0.1. This makes a future
editor (v1.x+) buildable without changes to the renderer interface. See
section 7.

**KSP-style readiness** — Transforms in the interface use double precision
for translation, and the view structure includes a hint about scene scale.
This makes a future large-scale simulation buildable without changes to
the interface. See section 9.

**Editor-style picking** — The renderer interface optionally produces an
"entity ID buffer" alongside the color buffer. This is dormant in v0.x
(the buffer is not requested) but the interface accepts the option, so
adding picking later does not break the interface.

**Hot-reload readiness** — Assets are referenced by stable handle, and the
renderer does not memoize raw pointers into asset storage. This makes
asset hot-reload buildable later without renderer changes.

These four decisions are the entirety of the forward-compatibility work
in v0.x. Anything beyond this is deferred until the corresponding feature
is actually being built.

---

## 12. What Is Explicitly Deferred

To prevent scope creep and the recurring temptation to over-design, the
following concerns are explicitly **not** addressed in v0.x and will be
revisited only when their respective milestone is reached.

- Scripting (any language) — deferred until post-v1.0.
- Networking — deferred indefinitely; out of v1/v2 scope.
- Physics — deferred until v1.x. The interface for physics is not
  designed in v0.x; it will be designed when needed.
- Skeletal animation — deferred. Static meshes only in v0.x.
- Audio spatialization — deferred. Basic 2D audio only in v0.x.
- Asset pipeline (compilation, packaging, optimization) — deferred. Raw
  asset loading only in v0.x.
- Multi-window — deferred until v2.x at earliest.
- Hot-reload of code (not assets) — deferred indefinitely.
- Console / mobile platforms — out of scope.

Refusing to design for these now is not a claim that they don't matter;
it is a claim that designing for them now would require speculation that
is more likely to be wrong than right.

---

## 13. Decision Records

Significant architectural decisions are recorded as Architecture Decision
Records (ADRs) in the `docs/adr/` directory of the meta repository. Each
ADR captures:
- The context that prompted the decision
- The alternatives considered
- The decision made
- The consequences accepted

ADRs are write-once. If a decision is later reversed, the original ADR is
not edited; instead, a new ADR is written that supersedes it and
references the original. This produces an honest history of how the
architecture evolved, which is useful both for the project's future
maintainer and for anyone trying to understand a non-obvious choice.

The decisions captured in this document are summarized in ADRs; the
canonical statement of each decision lives in this document, and the ADRs
provide the historical context.

---

## 14. Reading Order for Newcomers

For a person joining the project (including the author returning after a
break), the recommended reading order is:

1. **This document** — to understand the philosophy and the high-level
   structure.
2. **[`MODULES.md`](MODULES.md)** — to understand what each repository
   contains and how the responsibilities are divided.
3. **[`ROADMAP.md`](ROADMAP.md)** — to understand the current milestone
   and what the engine can and cannot do today.
4. **[`BOOTSTRAP.md`](BOOTSTRAP.md)** — to set up a working development
   environment.
5. **[`CONTRIBUTING.md`](CONTRIBUTING.md)** — to understand the
   day-to-day workflow (branches, commits, reviews).
6. **[`INTERFACES.md`](../liara-interfaces/INTERFACES.md)** (in the
   `liara-interfaces` repo) — required reading before modifying anything
   that crosses a module boundary.

Reading the entire codebase before contributing is not necessary. Reading
this document, however, is.
