---
title: Changing more than one repository
description: The five-step procedure for a change that crosses the contract, and why it is deliberately slow.
sidebar:
  order: 6
---

Some changes span repositories. A new function in `liara-interfaces` needs implementations in `liara-core` and `liara-renderer`, and consumption in the launcher.

## The procedure

**Open the change in `liara-interfaces` first.** The interface change is the contract, and it is reviewed and merged on its own.

**Tag a release of `liara-interfaces`**, a minor bump under the rules in [What breaks, and what does not](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/breaking-changes/#the-table), or a patch bump below 1.0.0, since release-please is configured to shift both down one level before then.

**Open the consumer changes**, each an independent pull request in its own repository, referencing the new interface version in its own `manifest.json`.

**Update the manifests** so that the compatibility is recorded where everything else reads it. There is no separate matrix file, and [ADR 0006](../../adr/0006-manifest-as-compatibility-source-of-truth/) is why.

**Update the launcher**, if it is affected, in a pull request of its own.

For a breaking change, the order inverts at the end: the consumer pull requests are opened at the same time as the interface one, and the interface one merges last, once every consumer is ready. Otherwise the consumers sit broken between the interface change and their adaptation.

## Why it is this slow

The procedure has five steps on purpose. Cross-repository changes are friction, and that friction is what discourages a frivolous interface change. [ADR 0001](../../adr/0001-multi-repository-layout/) is the argument in full: a boundary that can be crossed by editing one line is a boundary that gets crossed at 2am with a good reason.

The cost is real, and on a day when the work is obvious and the ceremony is not, it is the thing standing in the way. It is still cheaper than the alternative, which the previous engine demonstrated.

## Keeping the set findable

Pull requests that belong to one coherent change reference each other in their descriptions, as `Part of: liara/#42`. That is what makes the set discoverable later, when the question is why three repositories moved on the same afternoon.
