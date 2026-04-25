# Roadmap

> The path from an empty repository to Liara Engine 2.0, with explicit
> exit criteria for each milestone. This document is intentionally
> precise about Phase 0 through v1.0 and intentionally vague about
> what comes after; that vagueness is a feature, not a gap.

---

## 1. How to Read This Document

This roadmap is structured around **milestones**, not dates. Each
milestone has a defined scope, a Definition of Done, and an explicit
list of what is **not** in scope (the "non-goals" section is as
important as the "goals" section).

The document is divided into three zones of decreasing precision:

- **Phase 0 (Bootstrap)** is specified down to the file level. It
  must be completed entirely before v0.1 begins.
- **v0.1 through v1.0** are specified at the feature level. Each
  version has an explicit scope and a Definition of Done.
- **Post-v1.0** is specified only at the thematic level. Detailed
  scope for v1.x and v2.0 will be defined when v1.0 is reached, with
  the benefit of having actually used the engine.

This stratification reflects an honest assessment of what can be
planned reliably and what cannot. A solo project's priorities six
months out are heavily shaped by what is learned in the first six
months. Pretending otherwise produces detailed roadmaps that get
shredded the moment they meet reality.

---

## 2. Cadence Philosophy

The project does not follow a calendar-based release cadence. There
is no "v0.3 in March". A version ships when its Definition of Done
is met, the CI is green, and the documentation is updated — not
before, not on a deadline.

This choice acknowledges the reality of solo development under
TDAH/TSA constraints: hyperfocus periods may produce two releases in
a week, and academic or external commitments may produce a month of
silence. Both rhythms are normal. The project does not stress about
the silence and does not artificially slow down the bursts.

Three corollaries follow from this.

**Each version's scope is bounded.** A milestone is sized so that a
single hyperfocus session can finish it. If a version starts to feel
like it will not complete in one focus burst, it is too big and is
split. Open-ended versions invite abandonment; bounded versions
finish.

**Pauses are not failures.** The roadmap explicitly grants permission
to not touch the project for weeks or months. The next focus burst
picks up where the previous one left off, with the milestone document
as the bookmark. There is no concept of "falling behind".

**Definitions of Done are concrete.** The DoD for each version
consists of objectively verifiable items: tests pass, binary runs,
documentation page exists. There is no DoD item that reads "feature
X feels good"; either it works or it doesn't.

**Internal developer tools accompany features.** As the engine gains
capabilities, the developer tools that make those capabilities
observable accompany them in the same version. Tools are not a
separate workstream that lags behind; they are part of the Definition
of Done for each feature they apply to. A new subsystem without a way
to inspect its state is incomplete. All developer tools are
conditionally compiled out of release builds, or are hidden behind an
explicit debug toggle, so that they do not appear in shipped games.

---

## 3. Versioning Conventions

The meta repository (`liara`) versions itself separately from the
module repositories. When this document refers to "v0.3" without
qualification, it means the meta repository at version 0.3. Module
versions corresponding to a meta release are recorded in the
compatibility matrix.

Pre-1.0 versions follow the convention that minor bumps may include
breaking changes. Patch bumps are reserved for bug fixes and
documentation. Once v1.0 ships, the meta repository follows strict
semantic versioning.

Module repositories (`liara-core`, `liara-renderer`, etc.) follow
strict semantic versioning from day one. A breaking change to a
module is always a major version bump, regardless of whether the
module is at 0.x or 1.x.

---

## 4. Phase 0: Bootstrap

Phase 0 is not a version of the engine. It is the work of putting
in place the infrastructure on top of which versioned engine work
will happen. Phase 0 produces no engine code; it produces an
environment in which engine code can be productively written.

Phase 0 is treated as a single, atomic unit. It is "complete" only
when **every** item below is in place. Skipping items "for now and
adding them later" is what produced the original Liara's
infrastructure debt; this time, the discipline is upfront.

### Repositories Created

The following repositories exist on the `liara-engine` GitHub
organization:

- `liara` (meta)
- `liara-interfaces`
- `liara-core`
- `liara-renderer`
- `docs-shared`
- `.github` (organization-level)

Each module repository contains a minimal stub: a CMakeLists.txt
that compiles, a README that points to the meta repository's docs,
LICENSE, .gitignore, .editorconfig, .clang-format, .clang-tidy,
and a placeholder test that passes.

The `liara-editor` and `liara-physics` repositories are **not**
created in Phase 0. They will be created when their first code is
written.

### Documentation in Place

The meta repository contains, at minimum, the following documents,
all written and reviewed:

- `README.md`
- `docs/ARCHITECTURE.md`
- `docs/MODULES.md`
- `docs/ROADMAP.md`
- `docs/CONTRIBUTING.md`
- `docs/CODE_STYLE.md`
- `docs/TOOLING.md`
- `docs/BOOTSTRAP.md`
- `docs/adr/0001-*.md` through whatever ADRs are needed to capture
  the major decisions taken so far.

The `liara-interfaces` repository contains its own `INTERFACES.md`.

The `docs-shared` repository contains the navbar HTML/CSS/JS and
the Doxygen and mdBook templates that include it.

### CI Pipelines Operational

Each module repository has GitHub Actions workflows that, on every
push and pull request:

- Build on Linux (GCC and Clang) and Windows (MSVC).
- Run clang-format checks.
- Run clang-tidy.
- Run the test suite (even if it is just one placeholder test).
- Generate Doxygen documentation.
- On pushes to main, deploy the documentation to that module's
  `gh-pages` branch under a `/dev/` subdirectory.

The reusable workflows that implement these steps live in the
`.github` organization-level repository and are invoked from each
module's workflows.

### Release-Please Configured

Each repository has `release-please` configured to track conventional
commits and produce release PRs automatically. The configuration is
in `.release-please-manifest.json` and `release-please-config.json`
at each repository's root.

### Workspace Bootstrap Working

The meta repository contains a `scripts/setup-workspace.sh` script
that:

- Clones each module repository into a `workspace/` directory.
- Generates a top-level `CMakeLists.txt` that uses `add_subdirectory`
  to include each module.
- Configures CMake with the appropriate presets.
- Produces a `compile_commands.json` for clangd consumers.

The script's success criterion: starting from a fresh clone of the
meta repository on a system with the required dependencies (Vulkan
SDK, CMake, vcpkg, compilers), running `./scripts/setup-workspace.sh`
produces a buildable workspace in under five minutes.

### Documentation Hub Live

The meta repository's `gh-pages` branch hosts the documentation hub
at `liara-engine.github.io`. The hub:

- Displays a landing page with the project's name and description.
- Includes the shared navbar (consumed from `docs-shared`).
- Provides a version dropdown (initially listing only `dev`).
- Provides a module dropdown (listing the four initial modules).
- Links to each module's documentation on its respective `gh-pages`.

In Phase 0, the linked module documentation pages may be empty
placeholders. The point is that the navigation infrastructure works.

### Compatibility Matrix Initialized

The meta repository contains `compatibility.toml` listing the
expected versions of each module. In Phase 0, all modules are at
0.0.0 and the matrix entry is a single combination.

### Definition of Done for Phase 0

Phase 0 is complete when:

- [ ] All listed repositories exist and are configured.
- [ ] All listed documents are written.
- [ ] CI is green on every repository (even if testing only stubs).
- [ ] Documentation is generated and deployed to gh-pages.
- [ ] The hub at `liara-engine.github.io` is accessible and renders
      with the navbar.
- [ ] The workspace bootstrap script runs successfully on a clean
      Arch Linux machine.
- [ ] The workspace bootstrap script runs successfully on a clean
      Windows 11 machine with Visual Studio 2022.
- [ ] An ADR exists for each major decision: multi-repo layout, C
      ABI interfaces, ECS-from-scratch, Vulkan-Hpp, etc.

When all boxes are checked, the meta repository is tagged `v0.0.0`
and Phase 0 ends.

---

## 5. v0.1 — Hello Triangle

The first real version of the engine. The goal is not to render
something impressive; the goal is to **prove that the modular pipeline
works end to end**. Every architectural decision is exercised: the C
interfaces, the module loader, the renderer module's Vulkan
initialization, the launcher's composition of core and renderer, the
cross-platform build.

If v0.1 ships, the project's hardest architectural risk is behind it.
Every subsequent version is "just" adding features.

### Scope

The launcher (in the `liara` meta repository) initializes the core
module, which in turn initializes the renderer module via the C
interface, which renders a single hardcoded triangle to a window and
presents it. The window has a standard close button that triggers a
graceful shutdown propagated through the core to the renderer.

The triangle is hardcoded in the launcher: three vertices, a vertex
shader, a fragment shader. There is no ECS yet, no asset loader, no
math library beyond what the renderer needs internally.

The renderer initializes Vulkan with validation layers enabled in
debug builds. It uses Vulkan-Hpp for type safety and VMA for memory
management.

### Out of Scope

- ECS (v0.2)
- Asset loading (v0.3)
- Lighting (v0.4)
- Input handling beyond window close (v0.5)
- Audio (v0.5)
- Multiple objects, multiple frames worth of differing content
- Configurable shaders
- Editor anything

### Definition of Done

- [ ] Launcher builds and runs on Linux (GCC and Clang).
- [ ] Launcher builds and runs on Windows (MSVC).
- [ ] A window appears with a triangle rendered in it.
- [ ] The window is closable and the application shuts down cleanly.
- [ ] No Vulkan validation errors are reported in debug builds.
- [ ] The interface version negotiation between core and renderer
      is exercised: a contrived major-version mismatch produces a
      clear error and refuses to load.
- [ ] Tests pass on all platforms.
- [ ] User-facing documentation includes a "Build Hello Triangle"
      page.
- [ ] Compatibility matrix updated.

---

## 6. v0.2 — ECS and Math

With the modular pipeline proven, this version introduces the engine's
data model. The ECS becomes real, the math types live in headers, and
the render packet pattern is exercised for the first time.

### Scope

The core implements the ECS: entity allocation with generational
handles, sparse-set component storage, a basic query API
(`view<T1, T2, ...>`-style iteration), and entity lifecycle (create,
destroy, batch destroy).

The math types in `liara-interfaces` are populated:
`liara_vec2_t`, `liara_vec3_t`, `liara_vec4_t`, `liara_mat3_t`,
`liara_mat4_t`, `liara_quat_t`, `liara_transform_t`. The core
implements the math operations on these types.

The render packet structure in `liara-interfaces` is finalized for
v0.x usage: views, drawables, lights (placeholders for now). The
core builds a render packet each frame from ECS state. The renderer
consumes the packet.

The launcher's hardcoded triangle is replaced by code that creates a
few entities with `Transform` and `MeshRenderer` components, with the
mesh being a procedurally generated cube. A camera entity provides
the view. The user sees multiple cubes in the scene and a camera that
rotates around them automatically.

### Internal Developer Tools

This version introduces the first developer tools, since v0.2 is the
first version that produces data worth observing.

The renderer integrates ImGui, prepared during v0.1 but exercised
here for the first time. The engine displays a basic stats overlay
showing FPS, frame time, and ECS entity count. An ImGui-hosted log
console consumes the logger's in-memory ring buffer and displays log
entries in real time, with filtering by level and by source. A
settings inspector lists all registered settings, displays their
current values, and allows them to be modified at runtime.

These tools establish the pattern that every subsequent subsystem
will follow: when a feature is added, the developer tools that make
it observable are added in the same version.

### Performance Note

A benchmark is added to `liara-core`'s test suite: instantiate 10,000
entities with `Transform` and `MeshRenderer`, iterate over them, and
report µs per iteration. This benchmark runs in CI and its result is
recorded. The number itself does not gate the milestone, but its
existence does.

### Out of Scope

- Loading meshes from files (v0.3)
- Loading textures from files (v0.3)
- Materials beyond a hardcoded color (v0.4)
- Lighting computation (v0.4)
- ECS scheduler (parallel execution of systems) — deferred until
  there is a measurable need
- Component relations, observers, prefabs — deferred to v1.x

### Definition of Done

- [ ] ECS module is implemented in `liara-core`, with tests covering
      entity creation/destruction, generation counters,
      component add/remove, queries.
- [ ] Math types and operations are implemented and tested.
- [ ] Render packet structures are defined in `liara-interfaces`.
- [ ] The core builds a render packet from ECS state each frame.
- [ ] The renderer consumes the packet and draws the entities.
- [ ] Multiple cubes appear in the scene with distinct positions.
- [ ] A camera rotates around them.
- [ ] ECS benchmark runs in CI and results are recorded.
- [ ] ImGui stats overlay displays FPS, frame time, and entity count.
- [ ] ImGui log console displays logger output in real time with
      level and source filtering.
- [ ] ImGui settings inspector lists registered settings and allows
      runtime modification.
- [ ] User documentation includes a "Hello ECS" tutorial.

---

## 7. v0.3 — Assets

This version introduces persistent assets: meshes loaded from glTF
files, textures loaded from PNG/JPG, shaders compiled from GLSL to
SPIR-V at build time. The asset manager in `liara-core` becomes real.

### Scope

The core implements the asset manager: handles for meshes, textures,
and shaders, with reference counting and explicit lifetime
management. Assets are loaded via dedicated functions
(`liara_core_asset_load_mesh`, etc.) that return handles.

glTF support uses cgltf or tinygltf (TBD). The first supported
features are static meshes (no skeletal animation), with embedded or
external textures. Materials in glTF are read but only the
`baseColorTexture` is honored at this stage.

Texture loading uses stb_image. Supported formats: PNG and JPG.
Mipmaps are generated automatically.

The launcher's procedural cubes are replaced by real glTF assets
loaded from disk. A small selection of test assets ships with the
engine (a textured cube, a Suzanne head, a simple environment).

Shader hot-reload: in development builds, the engine watches the
shader source files and reloads pipelines when they change. This is
a quality-of-life feature for the engine's development itself, not a
shipped capability.

### Internal Developer Tools

The asset manager gains an ImGui asset browser: a panel listing all
loaded assets organized by type (meshes, textures, shaders), with
each asset's path, reference count, memory footprint, and load
timestamp. This panel is the primary tool for diagnosing asset
lifetime issues during development — leaked references, accidental
reloads, unexpected eviction.

A small "asset preview" capability is added: clicking a mesh or
texture in the browser shows a preview, useful for verifying that
the loaded asset matches expectations.

### Out of Scope

- Multiple textures per mesh (v0.4 with materials)
- glTF skeletal animation (deferred indefinitely or to v1.x)
- KTX2 compressed textures (v1.x)
- Asset preprocessing pipeline (v2.x)
- Asynchronous asset loading (v0.x will load synchronously; async
  is added later when needed)

### Definition of Done

- [ ] glTF static meshes load correctly.
- [ ] PNG and JPG textures load with mipmaps.
- [ ] Shader hot-reload works in development builds.
- [ ] The asset manager handles reference counting correctly
      (verified by tests).
- [ ] ImGui asset browser displays loaded assets with their type,
      reference count, memory footprint, and a basic preview for
      meshes and textures.
- [ ] The demo scene loads three different glTF models with
      textures.
- [ ] User documentation includes "Loading Your First Asset".

---

## 8. v0.4 — Lighting and Materials v1

The first non-trivial rendering. Lighting calculations, multiple
texture slots, basic materials. The scene starts to look like a
scene.

### Scope

The renderer implements forward rendering with Blinn-Phong lighting.
Supported light types: directional (one), point (multiple, with a
hardcoded limit like 32). Lighting parameters are exposed in the
render packet through a `liara_light_t` structure.

A material system replaces the v0.3 hardcoded color: each mesh
references a material, and a material has albedo texture, normal
map, optional metallic/roughness map (used as a single greyscale
combined map). The system is **not** PBR; it is a simple Blinn-Phong
material parameterized by textures.

Transparency is supported in a basic way: materials may declare
themselves as `OPAQUE` or `BLEND`, and `BLEND` materials are sorted
back-to-front for rendering. Alpha cutout is supported via a
threshold.

The demo scene is updated: a textured environment with multiple
meshes, a directional light producing simple shading, and a few
point lights with attenuation.

### Internal Developer Tools

This version introduces the debug rendering subsystem mentioned in
[`MODULES.md`](MODULES.md): lines, wireframes, axis-aligned bounding
boxes, frustums, and oriented bounding boxes can be submitted by any
system through the debug primitives API in `liara-interfaces`, and
are drawn in a dedicated pass after the main scene.

Light sources gain debug visualization: directional lights display
their direction with an arrow, point lights display their attenuation
radius as a translucent sphere, the active camera frustum can be
overlaid (useful when debugging culling). All visualizations are
toggled through the developer overlay.

The stats overlay is augmented with a per-stage timing breakdown:
CPU update time, render packet build time, GPU submit time, present
time. This is the first appearance of profiling data; a more
sophisticated profiler with timeline view is deferred to v1.x.

A material inspector panel allows the active material's parameters
to be modified at runtime, useful for tweaking lighting behavior
without recompilation.

### Out of Scope

- Shadow mapping (v1.x)
- PBR (v1.x)
- Image-based lighting (v1.x)
- Deferred rendering (v1.x or v2.x)
- Anti-aliasing beyond what Vulkan does by default
- Post-processing

### Definition of Done

- [ ] Materials with albedo, normal, and combined metallic/roughness
      texture slots work.
- [ ] One directional light and multiple point lights are rendered
      with correct shading.
- [ ] Transparency (blend and cutout) works.
- [ ] The demo scene shows a visually convincing scene.
- [ ] Debug rendering primitives (lines, AABBs, OBBs, frustums) are
      submittable through the public interface and drawn correctly.
- [ ] Light source debug visualization works (direction arrows,
      attenuation spheres).
- [ ] Stats overlay shows per-stage timing breakdown.
- [ ] Material inspector allows runtime modification of material
      parameters.
- [ ] User documentation includes a "Lighting Your Scene" page.

---

## 9. v0.5 — Input and Audio

The engine becomes interactive. Input events flow from the platform
to the ECS. Sounds play.

### Scope

Input handling lives in `liara-core` (not the renderer). SDL3 input
events are translated to engine events that scripts and systems can
consume. The first supported devices are keyboard, mouse, and one
gamepad (via SDL3's gamepad API).

An input mapping layer allows logical actions ("jump", "move_forward")
to be bound to physical inputs (key codes, mouse buttons, gamepad
buttons/axes). Mappings live in TOML configuration files and can be
modified at runtime.

Audio uses miniaudio. The engine supports loading and playing WAV and
OGG files. Initial scope: simple 2D sounds (no spatialization),
volume control per source, and looping. A small number of
simultaneous sources is supported (8 or 16, configurable).

The demo scene becomes interactive: WASD moves the camera, the mouse
looks around, a sound plays when the player presses Space. This is
not a game, but every primitive a game needs is now present.

### Internal Developer Tools

Two new panels accompany the new subsystems.

The input visualizer shows the current state of all input devices in
real time: which keys are pressed, mouse position and button state,
gamepad axes and buttons. Below the physical state, the panel shows
the resolved logical actions, which is essential for debugging input
mapping configurations.

The audio panel lists active audio sources with each source's volume,
playback position, and loop state. Playback can be paused or stopped
from the panel, and recently completed sources are kept visible for a
few seconds to help diagnose timing issues.

The settings inspector from v0.2 gains a new category for input
mappings: bindings can be modified at runtime through the UI without
editing TOML files manually.

### Out of Scope

- 3D audio (spatialization, attenuation, doppler) — v1.x
- More than one gamepad — deferred
- Touch input — out of scope
- VR input — out of scope
- Audio mixing buses — v1.x

### Definition of Done

- [ ] Keyboard, mouse, and gamepad inputs are received by the
      engine.
- [ ] Logical input actions can be bound to physical inputs through
      configuration.
- [ ] WAV and OGG audio files play.
- [ ] The demo scene is interactive: camera movement and sound on
      action.
- [ ] Input visualizer panel shows physical input state and resolved
      logical actions.
- [ ] Audio panel lists active sources and allows pause/stop.
- [ ] Input mappings can be edited at runtime through the settings
      inspector.
- [ ] User documentation includes "Handling Input" and "Playing
      Sounds".

---

## 10. v0.6 — Packaging

Up to this point, the engine has been runnable from a developer
build. v0.6 makes it distributable: any Linux user can install it
from the AUR; any Windows user can extract a zip and run a sample.

### Scope

CMake `install` rules are written for every module. Headers, libraries,
and shaders are installed to standard locations. `find_package` works:
a third-party project that wants to use Liara can install Liara, then
write `find_package(LiaraCore REQUIRED)` and link.

A PKGBUILD is written for the AUR. The package depends on the Vulkan
SDK and the runtime libraries the engine needs. It installs the
launcher, the engine libraries, the headers (for users who want to
build games against the engine), and a sample game.

For Windows, a portable archive is produced: the launcher executable,
the runtime DLLs, the shaders, the sample assets. Users extract and
run; no installer required.

A second meta-package is created on the AUR: `liara-engine-dev`,
which includes the development headers and CMake config files. The
runtime package (`liara-engine`) is sufficient to run games built
with Liara.

A simple sample game is written for the demo: a small interactive
3D scene where the player walks around and triggers sounds. This is
the first end-to-end test of "user can build and ship a game with
Liara".

### Internal Developer Tools

This version is where the developer tools' release-build behavior is
verified. All tools introduced in v0.2 through v0.5 (stats overlay,
log console, settings inspector, asset browser, debug primitives,
input visualizer, audio panel, material inspector) are conditionally
compiled out of release builds via a CMake option, or are accessible
only behind an explicit debug toggle (e.g., F2) that is disabled by
default in release mode.

The shipped sample game does not display any developer overlay by
default. Verifying this is part of the version's Definition of Done.

A small "release diagnostics" panel remains accessible even in
release builds, gated behind a key combination that an end user is
unlikely to discover accidentally. This panel exposes the engine
version, GPU information, and a way to dump a crash report. It is
the only developer-facing tool that ships with the runtime.

### Out of Scope

- Flatpak or Snap packaging (deferred indefinitely)
- macOS packaging (out of scope)
- Windows MSI installer (deferred; portable archive is enough)
- Code signing (deferred)
- Auto-update mechanism (out of scope)

### Definition of Done

- [ ] CMake install works for all modules.
- [ ] `find_package` works in a third-party project.
- [ ] AUR PKGBUILD builds and installs successfully.
- [ ] Windows portable archive runs on a clean Windows 11 machine.
- [ ] Sample game runs from both the AUR install and the Windows
      archive.
- [ ] All developer tools (overlay, panels, debug primitives) are
      either compiled out of release builds or hidden behind an
      explicit toggle that is off by default.
- [ ] The shipped sample game shows no developer UI on launch.
- [ ] Release diagnostics panel works and is reachable through the
      documented key combination.
- [ ] User documentation includes "Distributing Your Game".

---

## 11. v0.7 through v0.9 — Dogfooding

These versions do not introduce major new features. They exist to
**use the engine to build something** and fix the friction
discovered along the way. Each version is one iteration: the engine
is used to build a small game, the rough edges are filed down, the
documentation is improved.

This phase is critical and is where most hobbyist engine projects
fail by skipping it. The engine that has not been used to build a
game cannot be claimed to be usable for building games.

### What Is Built

A small but complete 3D game — not a tech demo, a small game. The
suggested archetype: a "corridor" or "small explorable scene" where
the player walks around, interacts with objects, triggers events,
and reaches some end state. Concrete enough to be shippable, simple
enough to be finished.

The choice of game is not prescribed. The criteria are: it must
exercise mesh rendering, lighting, input, audio, and asset loading;
it must have a beginning and an end; it must be packagable for
distribution. A good rule of thumb: it should be the smallest game
that someone unrelated to the project would call "a game" rather
than "a demo".

### Per-Iteration Pattern

Each of v0.7, v0.8, v0.9 follows the same pattern:

1. Identify the next set of frictions.
2. Fix them in the engine (not in the game).
3. Document the fixes.
4. Update the sample game to use the improved APIs.
5. Tag the version.

The frictions are discovered by genuinely trying to build the game.
Hypothetical frictions are not in scope; if it does not actually
hurt while building the game, it does not need fixing in this phase.

### Definition of Done for the Phase

- [ ] A small 3D game has been built using only the public engine
      APIs.
- [ ] The game is shippable: it has a start, an end, sounds, lights,
      and a few interactive elements.
- [ ] The game is packaged and runnable from both the Linux and
      Windows distributions.
- [ ] Each friction discovered has been either fixed or
      explicitly recorded as known-and-deferred.
- [ ] The user documentation has been augmented with at least one
      "real" tutorial that takes a user from empty project to small
      game.

---

## 12. v1.0 — First Shippable Game

The first stable release. The promise is: a sufficiently motivated
developer can build a small 3D game with Liara, ship it on the AUR
or as a Windows archive, and not feel cheated.

### Scope

v1.0 is mostly a quality and stability release on top of v0.9. The
features are essentially those of v0.9; what changes is the
discipline.

- Documentation is complete: every public function in
  `liara-interfaces` has Doxygen documentation; the user
  documentation covers the full feature set with tutorials.
- Stability: no known crashes in normal operation. Vulkan validation
  is clean. Memory leaks are tracked.
- API freeze: from v1.0 forward, the meta repository follows strict
  semantic versioning. Breaking changes require a major version
  bump.
- Compatibility matrix is finalized for the first stable
  combinations of module versions.

### What v1.0 Is Not

- It is not feature-complete.
- It is not the editor.
- It does not have PBR, shadow mapping, or post-processing.
- It does not have scripting.
- It does not have physics.

It is "first usable", not "first complete". This distinction is
honored in the version number and in the marketing.

### Definition of Done

- [ ] Sample game from v0.9 is finished and polished.
- [ ] Sample game is published on the AUR and as a Windows archive.
- [ ] Every public symbol in `liara-interfaces` is documented.
- [ ] User documentation covers all v0.x features with tutorials.
- [ ] No known crashes in the sample game.
- [ ] Memory leak detection is clean for the sample game.
- [ ] All module repositories are tagged at their respective
      compatible versions.
- [ ] Meta repository tagged `v1.0.0`.
- [ ] A blog post or release note describes what v1.0 is and is not.

---

## 13. Post-v1.0 — Themes Only

The roadmap deliberately becomes vague at this point. The features
in v1.x and v2.x will be defined when v1.0 ships, with the benefit
of having actually used the engine and learned what is missing.

The themes below are aspirational and unordered. They will be
prioritized (and possibly reshuffled) at v1.0.

### v1.x Themes

**Editor.** The single largest post-v1.0 effort. A scene editor with
a scene viewport, hierarchy, inspector, asset browser, play/pause
controls, and scene save/load. This is approximately what Unity
shipped as its first editor in 2005 and is years of work, but it is
the headline feature of the v1.x line. The interfaces are already
designed for this (see `ARCHITECTURE.md` sections 7 and 11).

**Scripting.** Hot-reloadable scripting for game logic. Initial host
language: C++ via shared libraries that are reloaded when changed.
Subsequent host languages (Lua, AngelScript, others) added as
plugins to the scripting host module. The scripting interface is
designed for multi-language from the start, even if only C++ is
implemented first.

**Physics.** Rigid body dynamics, collision detection. The choice
between custom and wrapping an existing library (Bullet, Jolt,
PhysX) is deferred until this milestone.

**Advanced rendering.** PBR, shadow mapping (cascaded), image-based
lighting, post-processing, anti-aliasing options. These are added
incrementally, each as its own minor release.

**Asset pipeline.** A preprocessing step that converts source assets
(glTF, PNG) into engine-native formats (KTX2 textures, optimized
mesh layouts) at build time. Faster load, smaller memory.

**Animation.** Skeletal animation for characters. Morph target
animation. Animation graphs.

**Advanced developer tools.** A timeline-style profiler with
hierarchical CPU and GPU sampling, replacing the per-stage breakdown
of v0.4. A scene tree inspector that supersedes v0.x's ad-hoc
panels. A frame capture tool that snapshots a complete frame for
later analysis. These tools become valuable when the engine's
complexity exceeds what the v0.x developer overlays can usefully
expose.

### v2.0 Theme

**Production-ready.** v2.0 is the version at which a sufficiently
motivated developer with significant time can attempt an ambitious
project — something on the scale of a small commercial indie game,
or a complex simulation experiment. v2.0 is not the engine that
ships KSP; it is the engine in which a determined developer can
attempt a KSP-like and not run into engine-level walls.

The criteria for v2.0 will be defined when the v1.x line approaches
maturity. They will likely include: a functional editor, working
scripting in at least two languages, a physics module, the full
rendering feature set, and a polished asset pipeline.

### What v2.0 Is Explicitly Not

- A general-purpose engine competing with Unity, Unreal, or Godot.
- A fully featured production tool with marketing, asset store, and
  community templates.
- A finished product. Even at v2.0, the engine is a personal
  learning project that happens to be usable; that frame does not
  change.

---

## 14. Living Document

This roadmap is a living document. Versions that have shipped are
moved to a "Completed" section (or annotated as completed in place,
TBD by the author). Versions that have not shipped may be revised
in light of experience.

Revisions follow a discipline:

- A version's scope may shrink (features may be moved to a later
  version) but should not silently grow. Adding scope to a version
  in flight is the start of vaporware.
- A version's Definition of Done may become more precise, but items
  may not be removed without justification.
- The post-v1.0 themes section is freely revisable; that is its
  purpose.

The roadmap is reviewed at the end of every shipped version. The
review either confirms the next version's plan or revises it.
