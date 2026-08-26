# Architecture Decision Records

Each file here records one decision: what was decided, what else was on the table, and what the decision costs. They are append-only. A decision that no longer holds is not edited or deleted — a new ADR supersedes it, and the old one is marked as superseded, because the reasoning that turned out to be wrong is often more useful than the reasoning that turned out to be right.

Most of the Phase 0 records are retrospective: the decision was made while bootstrapping and written down afterward. That is stated rather than hidden. A retrospective ADR records why a decision still holds, which is a different and more honest claim than a reconstruction of why it was made.

| ADR                                                       | Title                                                       | Status   |
|-----------------------------------------------------------|-------------------------------------------------------------|----------|
| [0001](0001-multi-repository-layout.md)                   | Multi-repository layout                                     | Accepted |
| [0002](0002-c-abi-as-the-inter-module-contract.md)        | A C ABI as the inter-module contract                        | Accepted |
| [0003](0003-the-host-composes-modules.md)                 | The host composes modules                                   | Accepted |
| [0004](0004-module-boundaries.md)                         | Module boundaries: what stays in the core                   | Accepted |
| [0005](0005-version-encoding-and-compatibility.md)        | Version encoding and compatibility semantics                | Accepted |
| [0006](0006-manifest-as-compatibility-source-of-truth.md) | `manifest.json` as the single compatibility source of truth | Accepted |
| [0007](0007-cmake-presets-and-vcpkg.md)                   | CMake with presets, and vcpkg in manifest mode              | Accepted |
| [0008](0008-two-tier-ci.md)                               | Two-tier CI, and reusable workflows                         | Accepted |
| [0009](0009-in-house-ecs.md)                              | An in-house ECS rather than an existing library             | Proposed |
| [0010](0010-vulkan-without-abstraction-layer.md)          | Vulkan directly, without a rendering abstraction layer      | Proposed |

New tooling requires an ADR (`TOOLING.md` section 1), with one exception: the bootstrapping phase, whose policy is in `ROADMAP.md` section 4.
