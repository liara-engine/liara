# ADR 0008 — Two-Tier CI, and Reusable Workflows Over Composite Actions

- **Status**: Accepted
- **Date**: 2026-08-26
- **Deciders**: Antoine

## Context

Six repositories need CI that answers two questions which are not the same question. "Does this compile and pass its tests" gets asked dozens of times a day on a branch that is nowhere near finished, and is only useful if the answer comes back in a couple of minutes. "Does this still compose with every other module, on both platforms, in both linkages" gets asked once per pull request and is worth waiting for.

I designed the second one first: a single pipeline on pull requests and `main`, assembling the workspace and building the full matrix. Then I looked at what that means day to day and the answer is nothing at all — a branch gets no feedback until a pull request exists. Which leaves two options, both bad: open the pull request early just to see a build, and run the heavy matrix on work-in-progress; or work blind and hit a wall of failures at review time.

Sizing the single pipeline down instead does not work either, because then nothing ever checks composition, which is the property the multi-repository layout of ADR 0001 exists to protect.

There was also the question of where the YAML lives, since whatever the pipelines do, six repositories run substantially the same ones.

## Decision

Two tiers, with different build models.

The **branch tier** runs on every push outside `main`. It builds the repository on its own, on one Linux preset, against the `liara-interfaces` version that repository's own manifests declare, and runs clang-format. It fetches the preset template from the meta repository rather than carrying a copy. It is not a required check.

The **integration tier** runs on pull requests, on `main` and on tags. It assembles the workspace superbuild with the repository under test substituted for its clone, and builds the full matrix: Linux under GCC and Clang, Windows under MSVC, both linkages, and the runtime-loading configuration. It runs clang-tidy against the workspace's compilation database. This is what branch protection requires.

Both tiers live in the organization's `.github` repository as reusable workflows, called by a thin caller in each repository. Composite actions are used for step sequences shared between workflows, not for pipelines.

Every step of both tiers emits a report fragment, and a final job renders them into one sticky pull request comment.

## Alternatives Considered

**One tier, pull-request triggered only.** Where I started, and the reason this record exists. The failure mode is above: no feedback on a branch means premature pull requests or blind work, and both cost more than the second tier does.

**One tier, full matrix on every push.** Feedback everywhere and only one model to maintain. Rejected on cost and latency both: nine legs on every intermediate commit burns minutes for a signal that is mostly redundant, and a fifteen-minute answer to "does this compile" is not an answer anyone waits for.

**Standalone builds in both tiers.** Simpler, faster, and it is what the branch tier does. Rejected for the integration tier: a module built alone against an installed contract cannot notice it has broken a sibling, and inter-module breakage is the specific risk that comes with splitting a codebase across repositories. The workspace build is the only thing that answers that, and it costs a few minutes.

**Copy-pasting the workflow YAML into each repository.** A fix to one repository's CI would not reach the other five, and the drift stays invisible until a pipeline that was supposed to be identical behaves differently.

**Composite actions for the pipelines.** They cannot express a matrix or multiple jobs, which is most of what these pipelines are. They are still the right tool for the workspace-assembly step sequence that build and lint both need, and that is where I use them.

## Consequences

Two build models have to stay coherent. A change to how the workspace is assembled has to be mirrored in the standalone path, and if they diverge I get the worst possible outcome: a green branch tier and a red integration tier for reasons unrelated to the change.

The branch tier is deliberately incomplete and that has to stay explicit. No Windows, no shared libraries, no clang-tidy, no composition check. A green branch build means "this compiles", not "this is ready", and I need to keep reading it that way.

Which `liara-interfaces` a module builds against comes from its own `manifest.json` rather than from `main`. That is what makes the branch tier meaningful when the contract moves ahead of its consumers, which is the normal direction of travel here. It also means a wrong manifest produces a wrong build, and the compiler cannot tell me the manifest is the problem.

The reusable workflows are versioned by floating tags, so a fix reaches six repositories at once — and so does a mistake. Preview tags exist to test a workflow change before it floats.

The sticky comment is the interface to a run, and its budget is finite: nine legs of build logs would overflow GitHub's comment limit. A passing leg gets one line and only failures carry their output. A report that sends me back to the raw logs has failed at the one thing it is for.

The Zig and Rust cross-language tests run on exactly one leg, and that leg fails configuration if the toolchains are missing. Without that, "installed nowhere" and "installed everywhere" would look identical from outside.

## Revisit If

- The branch tier stops being fast enough to run on every push — which calls for narrowing it, not for widening the integration tier.
- Contributors other than me appear, which turns the branch tier's incompleteness from a known limitation into a trap, and probably makes it a required check.
- The two build models diverge in practice rather than in theory. The first time a green branch build hides an integration failure is the signal.
