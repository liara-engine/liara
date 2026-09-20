---
title: Adding and removing a module
description: The six steps that introduce a module, why the namespace is claimed before the repository exists, and what it costs to rename one.
sidebar:
  label: Adding and removing
  order: 12
---

## Adding one

**1. Claim the namespace.** Before any code: pick `<name>`, and use `liara/<name>/` and `liara_<name>_*` from the first line, even while the implementation still lives inside an existing repository. Free now, irreversible later.

**2. Justify the module.** A subsystem earns its own namespace when it has a boundary expressible as a C interface, and at least one of three things is true: it is not always needed and a headless tool can do without it, it is a plausible replacement target, or it drags in an external dependency nothing else needs. A subsystem failing all three belongs inside an existing module.

**3. Design the interface.** Add the headers to `liara-interfaces`. This is the expensive step and the one that must not be rushed, and it is also why a module's interface is designed when its first real implementation is written rather than years ahead. `liara-physics` is the standing example of the second half of that rule.

**4. Bump `liara-interfaces`.** A new namespace is purely additive, so it is a minor bump.

**5. Create the repository**, and not before this point. Copy the standard scaffolding, then register it in three places: the workspace orchestrator's module list, `modules-registry.json` so the documentation switchers know about it, and its own `manifest.json` so compatibility can be computed against it.

**6. Update these pages**, the dependency graph, and the cross-reference table.

Splitting an existing module follows steps 1 to 3 only, then moves files. That is the entire point of step 1: if the namespace was claimed early, a split is a repository move and consumers recompile unchanged.

## Removing or renaming one

Neither happens lightly. When a module's responsibilities shrink to nothing, or when its name stops describing what it does, the change is a major version event for the meta repository. The old repository is archived rather than deleted, the new one is created, and the migration path is recorded in an ADR.

The conservatism is about links rather than code. External references to a repository accumulate, breaking them costs something real even at this scale, and an archived repository still answers the URL somebody bookmarked two years ago.
