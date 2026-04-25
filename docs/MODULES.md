# Modules

> Concrete decomposition of the Liara Engine project into repositories,
> modules, and the boundaries between them. This document is the
> companion to [`ARCHITECTURE.md`](ARCHITECTURE.md): where
> `ARCHITECTURE.md` explains the philosophy of modularity, this
> document specifies what is actually built and where it lives.

---

## 1. Repository Map

The project is composed of the following repositories, all hosted under
the `liara-engine` GitHub organization:

| Repository          | Role                                       | Replaceable | Status  |
|---------------------|--------------------------------------------|-------------|---------|
| `liara`             | Meta repository, launcher, distribution    | No          | Phase 0 |
| `liara-interfaces`  | C ABI headers shared by all modules        | No          | Phase 0 |
| `liara-core`        | Engine foundation (ECS, math, assets, ...) | No          | Phase 0 |
| `liara-renderer`    | Reference Vulkan renderer                  | Yes         | Phase 0 |
| `liara-editor`      | Editor application (post-v1.0)             | No          | Deferred |
| `liara-physics`     | Physics module (post-v1.0)                 | Yes         | Deferred |
| `docs-shared`       | Shared navigation and doc templates        | No          | Phase 0 |
| `.github`           | Org-wide templates and shared actions      | No          | Phase 0 |

The "Replaceable" column indicates whether the module is designed to be
substitutable by an alternative implementation. Replaceable modules
expose their entire functionality through `liara-interfaces`;
non-replaceable modules may use richer C++ APIs internally.

The "Status" column indicates the planned introduction. "Phase 0" means
the repository must exist before v0.1. "Deferred" means the repository
is created when its first code is written, not before.

A visual representation of the dependency graph appears in section 9.

---

## 2. The Meta Repository: `liara`

The `liara` repository is the public face of the project and the
orchestrator of everything else. It does not contain engine code; it
contains the things that need to know about all modules at once.

### Purpose

The meta repository serves four functions. It is the **landing page** for
the project on GitHub: the README that a visitor reads first, the issue
tracker that catches general questions, the discussions board for
project-wide topics. It is the **launcher and distribution** point: the
small executable that loads the core, the AUR `PKGBUILD`, the Windows
installer or portable archive script. It is the **workspace orchestrator**:
the bootstrap script that clones and configures all other repositories
for local development. And it is the **compatibility matrix**: the
authoritative record of which versions of which modules work together.

### Contents

The repository contains, at minimum:

- A top-level README with project description, build status badges, and
  links to documentation.
- This entire `docs/` directory: the foundational documents
  (`ARCHITECTURE.md`, `MODULES.md`, `ROADMAP.md`, etc.) and the ADRs.
- A `launcher/` source directory containing the small standalone
  application that initializes the core and runs the standalone game
  loop. This is the executable that ships in the AUR package.
- A `scripts/` directory containing the workspace bootstrap script,
  release coordination scripts, and any other tooling that operates
  across repositories.
- A `compatibility.toml` file that lists, for each release of the
  meta repository, which versions of which modules are bundled together.
- A `packaging/` directory containing the Linux `PKGBUILD`, the Windows
  packaging script, and any other distribution artifacts.
- A `hub/` directory containing the documentation hub website (the
  static HTML/JS that becomes `liara-engine.github.io`).

### What It Does Not Contain

The meta repository does not contain engine logic, rendering code, ECS
implementation, or any other functionality that belongs in a module. It
also does not contain documentation generated from source code (Doxygen
output) — that documentation is generated and hosted by each module's
own GitHub Pages.

### Versioning Policy

The meta repository uses its own version number, distinct from the
versions of the modules it bundles. A release of `liara` 1.0.0 corresponds
to a specific combination of `liara-core`, `liara-renderer`, etc.,
specified in `compatibility.toml`. The user-visible "Liara Engine 1.0.0"
refers to a release of this repository.

The meta repository's versioning follows the engine's milestone roadmap.
Module repositories version independently, on their own cadence.

---

## 3. The Interface Repository: `liara-interfaces`

The `liara-interfaces` repository is the most version-sensitive piece of
the project. Every other module declares which version of
`liara-interfaces` it requires, and a breaking change to interfaces
ripples through every consumer.

### Purpose

This repository defines the C ABI contracts between modules. Nothing in
the project may communicate across a module boundary without going through
a header defined here. Modules that internally use richer C++ types
must wrap them at their public surface to expose only the C interface.

### Contents

The repository contains exclusively C headers, organized by module:

```
include/liara/
├── version.h           # Version macros and ABI version negotiation
├── types.h             # Common POD types (vec3, mat4, handles, etc.)
├── result.h            # Error reporting convention
├── allocator.h         # Memory allocator interface (caller-supplied)
├── core/
│   ├── module.h        # Generic module lifecycle (init, shutdown, version)
│   ├── world.h         # ECS-side opaque handles (entity, world)
│   └── events.h        # Input and lifecycle events
├── renderer/
│   ├── renderer.h      # Renderer module entry point and lifecycle
│   ├── render_target.h # Abstract render target type
│   ├── view.h          # View / camera structures
│   ├── packet.h        # Render packet structures
│   └── debug.h         # Debug primitive submission
├── physics/            # (post-v1.0)
└── scripting/          # (post-v1.0)
```

In addition to headers, the repository contains:

- A `CMakeLists.txt` exposing the headers as an `INTERFACE` library that
  consumers link against to gain include paths.
- A `vcpkg.json` stub (no external dependencies, but present for
  consistency with other modules).
- Tests that verify each header compiles standalone and is C-callable
  (compiled with both a C compiler and a C++ compiler in CI).
- The `INTERFACES.md` document specifying the rules for designing,
  evolving, and versioning interfaces.

### What It Does Not Contain

No implementation. No `.c` or `.cpp` files (other than the test files,
which are minimal and verify properties of the headers themselves). No
external dependencies (the headers may use only fixed-width integer
types from `<stdint.h>` and `<stddef.h>`).

### Versioning Policy

This repository follows **strict semantic versioning**, with the rules
spelled out in `INTERFACES.md`. The headline:

- **MAJOR** version bumps for any change that breaks ABI or source
  compatibility for consumers.
- **MINOR** version bumps for purely additive changes that do not affect
  existing consumers.
- **PATCH** version bumps for documentation, comments, or whitespace.

The version is encoded in `version.h` as preprocessor macros and is
checked at module load time via the function each module exports to
report its interface version.

---

## 4. The Core Repository: `liara-core`

The core is the foundation that every other module depends on. It owns
the data and the schedule; modules transform data on a schedule the
core dictates.

### Purpose

The core implements everything that is shared, mandatory, and not
replaceable. It is the answer to "what is always there, no matter what
modules are loaded?".

### Contents

The core repository implements:

**The ECS.** Entity allocation with generational handles, sparse-set
component storage, world container, query API, system scheduling. This
is hand-written from scratch (see `ARCHITECTURE.md` section 6 for
rationale).

**The math layer.** Vector, matrix, and quaternion types as plain C
structs (defined in `liara-interfaces`), with implementation functions
that operate on them. Internal computation may use a vendored copy of
GLM where convenient, but no GLM type ever crosses the module boundary.

**The asset manager.** Loading, lifetime management, and handle
allocation for engine-known asset types (meshes, textures, shaders,
audio). Asset format support is initially limited to glTF for models,
common formats via stb_image for textures, SPIR-V for shaders, and
common formats via miniaudio for audio. The asset manager does not
upload data to the GPU; that is the renderer's job. The asset manager
provides the renderer with CPU-side data and a stable handle.

**The logger.** A multi-threaded logging system with structured log
entries, multiple sinks (stdout, file, in-memory ring buffer for the
ImGui console), and runtime log level control. The architecture
follows what was prototyped in the previous engine, simplified.

**The settings system.** Type-safe key-value storage with serialization
to TOML, runtime change notifications, and category-based organization.
This carries forward the design from the previous engine, with the
file format changed from custom to TOML.

**The signal handler.** Cross-platform graceful shutdown handling for
SIGINT, SIGTERM, and Windows console events. Translates platform
signals into engine-internal shutdown events.

**The event system.** Internal pub-sub for engine events (entity
created, component added, asset loaded, etc.) and external input events
(keyboard, mouse, gamepad, window). Decoupled from rendering: input
events flow through the core and may be consumed by scripting, editor,
or game logic, not just by the renderer.

**The application loop primitives.** The `liara_core_step(dt)` function
that advances the simulation by one tick, along with the helper
functions that the launcher (or the editor, in editor mode) uses to
build a complete loop.

**The module loader.** The code that, at static-link time, registers
each compiled-in module's entry points and verifies their interface
versions. This is the seed for what will become a runtime dynamic
loader if the project ever moves to DSO modules.

### What It Does Not Contain

No rendering code. No platform-specific window creation (the windowing
abstraction is in core, but the actual SDL3 calls happen in a small
platform layer that is private to core). No editor-specific code. No
gameplay code. No tools.

### Internal Organization

The core's internal directory layout follows the categories above:

```
src/
├── ecs/
├── math/
├── assets/
├── logger/
├── settings/
├── signal/
├── events/
├── loop/
├── modules/
└── platform/
    ├── linux/
    └── windows/
```

This layout is internal to the core and may evolve. It is not part of
any contract.

### Versioning Policy

The core follows semantic versioning, but its compatibility constraints
are looser than `liara-interfaces`. A patch or minor change to the core
that does not affect any interface is invisible to other modules. A
major version bump of the core is rare and significant.

The core's version is independent of the meta repository's version.

---

## 5. The Renderer Repository: `liara-renderer`

The renderer is the reference implementation of the renderer interface.
It is the most complex module and the one whose replaceability matters
most.

### Purpose

The renderer takes render packets produced by the core and produces
pixels. It manages the GPU, the swapchain, render targets, pipelines,
and shaders. It does not know about the ECS, about gameplay, or about
the editor; it only knows about the data the core hands it each frame.

### Contents

The renderer repository implements:

**The Vulkan device layer.** Instance creation, physical device
selection, logical device creation, queue management, command pool
management. Built on `Vulkan-Hpp` and `VMA`.

**The swapchain manager.** Surface creation (delegating to the
windowing layer in core for the native handle), swapchain creation,
recreation on resize, image acquisition, and presentation.

**The render target abstraction.** Both swapchain-backed targets and
offscreen-texture-backed targets, exposed through the same opaque
handle to consumers. Image transitions, format negotiation, and
allocation strategy are encapsulated here.

**The pipeline cache.** Shader module loading, pipeline state
description, pipeline creation, caching of pipelines keyed by their
state. SPIR-V is consumed; the renderer does not compile GLSL itself
(that is done at build time by `glslc` invoked from CMake).

**The render passes.** The actual frame logic: receiving a render
packet, sorting by material/pipeline, issuing draw calls, handling
transparency, presenting to the swapchain.

**The debug rendering subsystem.** Lines, wireframes, AABBs, frustums,
and other primitives submitted by the core (or, post-v1.0, by the
editor for gizmos). Implemented as a separate pass with its own simple
pipeline.

**The ImGui integration.** ImGui is rendered by the renderer because
it is fundamentally a rendering operation. The integration is
designed so that the editor (post-v1.0) submits its UI through the
editor interface, and the renderer draws it. In v0.x, ImGui is used
only for the developer console and stats overlays.

### What It Does Not Contain

No ECS code. No gameplay logic. No window creation (the window is
created by the core's platform layer; the renderer receives a native
handle). No asset loading from disk (the renderer receives prepared
asset data from the core's asset manager). No input handling.

### Internal Organization

```
src/
├── device/
├── swapchain/
├── targets/
├── pipelines/
├── passes/
├── debug/
├── imgui/
└── platform/
shaders/
```

The `shaders/` directory contains GLSL source for the engine's built-in
shaders, compiled to SPIR-V at build time and either embedded in the
binary or shipped alongside it (controlled by a CMake option).

### Versioning Policy

The renderer follows semantic versioning. Its public contract is
defined entirely by `liara-interfaces`, so most changes (bug fixes,
performance improvements, internal refactoring) are invisible to
consumers and result in patch or minor bumps. Major bumps are rare and
correspond to changes in resource consumption, behavior in edge cases,
or removal of optional features.

The renderer's version is independent of both the core's version and
the meta repository's version. The compatibility matrix in the meta
repository tracks which combinations are tested.

---

## 6. The Editor Repository: `liara-editor`

The editor is the in-engine authoring tool that, post-v1.0, allows
scenes to be built without writing code. It is **not introduced until
the v1.x cycle**; the repository is created when the first editor code
is written.

### Purpose

The editor is the application a developer launches to build a game.
It hosts the engine, lets the developer place entities in the scene
visually, edit their components through an inspector, and save and
load scenes. It is the analog of the Unity Editor or the Unreal Editor.

### Status in v0.x

The editor does not exist in v0.x. Scenes are constructed in code or
loaded from JSON files written by hand. This is acknowledged as a
limitation; it is not a permanent state of affairs. The interfaces in
v0.x are designed so that introducing the editor in v1.x does not
require interface changes — see `ARCHITECTURE.md` section 7 and
section 11 for the specific design choices that enable this.

### Contents (Anticipated)

When the editor is built, the repository will contain:

- An application that owns its own window and swapchain, embeds the
  engine as a library, and drives the engine's tick manually.
- An ImGui-based UI: scene hierarchy, component inspector, asset
  browser, scene viewport (rendering the scene to a texture and
  displaying it in an ImGui panel), play/pause/step controls.
- Gizmo rendering for entity manipulation, integrated with the
  renderer's debug rendering subsystem.
- Scene serialization (load/save scenes as JSON or a custom format).
- Project management (create new project, set up directory structure,
  manage assets).

### Replaceability

Although the editor is, in principle, a swappable module like the
renderer, the practical question of someone writing an alternative
editor is unlikely. Editors are large pieces of software with
complex UI requirements, and the value of having an alternative
editor is questionable for a project of this scale. The editor
therefore is **not designed for replaceability**; it consumes the
engine through richer C++ APIs where convenient.

This is a deliberate exception to the modularity principle, made
because the cost of treating the editor as replaceable would be
high and the benefit nil.

---

## 7. The Physics Repository: `liara-physics`

The physics module provides collision detection and rigid body
dynamics. It is **not introduced until the v1.x cycle**.

### Status in v0.x

Physics does not exist in v0.x. Entities have transforms, but nothing
moves except by direct ECS manipulation. This is acknowledged as a
limitation.

When the physics module is introduced, its design will be informed by
experience using the engine to build a game without it. The interface
for physics is **not** designed in v0.x precisely because that design
would be premature.

### Contents (Anticipated)

When introduced, the physics module will likely include:

- A choice of integration: a custom implementation, or a wrapper around
  Bullet, PhysX, or Jolt. This decision is deferred until the milestone.
- Collision shapes (box, sphere, capsule, mesh, heightfield).
- Rigid body dynamics with constraints and joints.
- Raycasts and queries.
- Debug visualization integrated with the renderer's debug rendering
  subsystem.

### Replaceability

Physics is designed as a replaceable module. The interface in
`liara-interfaces/physics.h` will be the contract.

---

## 8. Auxiliary Repositories

### `docs-shared`

A small repository containing the shared navigation bar and
documentation templates that every module's published documentation
includes. The shared assets are:

- `navbar.html`, `navbar.css`, `navbar.js` — the navigation bar that
  appears at the top of every documentation page across all modules.
- `version.json` — list of available versions, consumed by the navbar's
  JavaScript to populate the version dropdown.
- `doxygen-header.html`, `doxygen-footer.html`, `doxygen-stylesheet.css`
  — Doxygen template files that integrate the navbar.
- `mdbook-theme/` — mdBook theme files that integrate the navbar.

Each module's documentation CI job downloads this repository at build
time and uses its templates when generating documentation. Updates to
the navbar are made here and propagate on the next documentation
generation in each module.

This repository is small, rarely changes, and follows simple
versioning: each release tag corresponds to a snapshot that is referenced
by module CI jobs. Modules pin a specific version to avoid surprise
breakage.

### `.github` (Organization-Level)

A special repository that GitHub recognizes as the source of
organization-wide defaults. It contains:

- `profile/README.md` — the organization's public landing page.
- `ISSUE_TEMPLATE/` — issue templates that apply to repositories
  without their own.
- `pull_request_template.md` — the default PR template.
- `workflows/` — reusable GitHub Actions workflows that other
  repositories invoke (build, lint, generate docs, deploy gh-pages).

Each module's CI consumes the reusable workflows from this repository,
so that updates to CI logic happen in one place. This is the same
discipline as `docs-shared`, applied to CI.

---

## 9. Dependency Graph

The static dependency graph between repositories is:

```
                         ┌─────────────────────┐
                         │   liara-interfaces  │
                         │  (C headers only)   │
                         └──────────┬──────────┘
                                    │
                  ┌─────────────────┼─────────────────┐
                  │                 │                 │
                  ▼                 ▼                 ▼
           ┌────────────┐    ┌──────────────┐  ┌──────────────┐
           │ liara-core │    │liara-renderer│  │liara-physics │
           │            │    │              │  │  (post-v1)   │
           └─────┬──────┘    └──────────────┘  └──────────────┘
                 │                  │                  │
                 └────────┬─────────┴────────┬─────────┘
                          │                  │
                          ▼                  ▼
                   ┌────────────┐     ┌──────────────┐
                   │   liara    │     │ liara-editor │
                   │ (launcher) │     │  (post-v1)   │
                   └────────────┘     └──────────────┘
```

Reading the graph:

- `liara-interfaces` depends on nothing.
- `liara-core`, `liara-renderer`, and `liara-physics` each depend only
  on `liara-interfaces`. They do **not** depend on each other.
- The `liara` meta repository (specifically the launcher) depends on
  `liara-core` and `liara-renderer` (and later, `liara-physics`),
  composing them into a runnable application.
- The `liara-editor` repository (post-v1.0) depends on the same set,
  composing them into the editor application.

The crucial property is that the renderer does **not** depend on the
core, and the core does **not** depend on the renderer. They are
siblings, communicating only through interfaces. This is what makes
either of them replaceable. If the core had a `#include` of a
renderer header, the architecture would be broken.

The auxiliary repositories (`docs-shared`, `.github`) are not in the
dependency graph because their consumption is at the CI level, not the
build level. They are pulled in by GitHub Actions, not by CMake.

---

## 10. Module Boundaries: What Crosses, What Doesn't

This section enumerates the data flows that cross module boundaries.
Anything not listed here should not cross.

### Core → Renderer (per frame)

The core sends the renderer a render packet describing what to draw
this frame. The packet contains, at minimum:

- A list of views, each with a camera, viewport, and target.
- A list of drawables per view, each with a transform, mesh handle,
  and material handle.
- A list of lights affecting the scene.
- A list of debug primitives to draw this frame.
- A list of UI draw commands (ImGui draw data, in v0.x).

This data is plain-old-data and does not retain ownership: once the
renderer has consumed the packet, it may discard it.

### Core → Renderer (lifecycle)

At startup, the core hands the renderer a native window handle (HWND,
Wayland surface, X11 window) and configuration parameters (preferred
GPU, validation layer enable, etc.). The renderer initializes itself
and reports success or failure.

At shutdown, the core requests the renderer to flush in-flight work and
release resources.

### Core → Renderer (resources)

When an asset is loaded by the core, it is uploaded to the GPU via a
renderer call. The core hands the renderer the CPU-side data and a
stable handle; the renderer associates the GPU-side resource with the
handle. Subsequent draw calls reference the asset by handle.

### Renderer → Core (callbacks)

The renderer reports events back to the core: surface lost, swapchain
resized, GPU error. These are reported through callback functions
registered by the core at renderer init.

### Core → Editor (post-v1)

The editor reads ECS state to populate the scene hierarchy and
inspector. The interface for this is **read-mostly**: the editor can
inspect any entity but mutates entities only through specific edit
commands that the core executes (so that undo/redo is possible).

### Editor → Renderer (post-v1)

The editor submits gizmo geometry through the debug rendering
interface, on top of the scene render. The editor also requests render
targets for its scene viewport panels.

---

## 11. Where Things Live: A Cross-Reference

For specific topics, the canonical location is:

| Topic                          | Repository           |
|--------------------------------|----------------------|
| ECS implementation             | `liara-core`         |
| ECS handle types (in API)      | `liara-interfaces`   |
| Math types (in API)            | `liara-interfaces`   |
| Math implementation            | `liara-core`         |
| Asset loading                  | `liara-core`         |
| GPU upload                     | `liara-renderer`     |
| Window creation                | `liara-core`         |
| Vulkan device                  | `liara-renderer`     |
| Render packet structure        | `liara-interfaces`   |
| Render packet construction     | `liara-core`         |
| Render packet consumption      | `liara-renderer`     |
| ImGui setup                    | `liara-renderer`     |
| Debug primitives API           | `liara-interfaces`   |
| Debug primitives drawing       | `liara-renderer`     |
| Logger                         | `liara-core`         |
| Settings (TOML)                | `liara-core`         |
| Standalone game loop           | `liara` (launcher)   |
| Editor loop                    | `liara-editor` (post-v1) |
| AUR PKGBUILD                   | `liara`              |
| Doxygen config per module      | each module repo     |
| User documentation (mdBook)    | `liara` (`docs/user/`) |
| ADRs                           | `liara` (`docs/adr/`) |
| Shared navbar                  | `docs-shared`        |
| Shared CI workflows            | `.github` (org-level) |

When a topic is added to the project, this table is updated.

---

## 12. Adding a New Module

The process for introducing a new module to the project is:

1. **Justify the module's existence.** A new module is added when
   functionality cannot reasonably fit in an existing module without
   bloating it, and when the new functionality has a clear boundary
   that can be expressed as a C interface.

2. **Design the interface first.** Add headers to `liara-interfaces`
   defining the module's public surface. This is the most expensive
   step and should not be skipped.

3. **Bump `liara-interfaces`.** Adding a new module is a minor version
   bump (additive). Update the version macros and tag a release.

4. **Create the module repository.** Follow the conventions for
   repository setup (see `CONTRIBUTING.md`).

5. **Update this document.** The new module appears in the repository
   map (section 1), the dependency graph (section 9), and the
   cross-reference (section 11).

6. **Update the compatibility matrix.** The meta repository's
   `compatibility.toml` learns about the new module.

7. **Update the launcher (if applicable).** If the new module is part
   of the standalone runtime, the launcher composes it into the loop.

The cost of adding a module is intentionally non-trivial. Modules
should be added because they earn their place, not because the project
is in a phase where adding modules feels productive.

---

## 13. Removing or Renaming a Module

Modules are not removed or renamed lightly. When a module's
responsibilities shrink to nothing, or when its name becomes
misleading, the change is treated as a major version event for the
meta repository. The old repository is archived (not deleted), the new
repository is created, and the migration path is documented in an ADR.

This conservatism reflects the reality that external links to
repositories accumulate over time and that breaking them is a real
cost, even for a project at this scale.
