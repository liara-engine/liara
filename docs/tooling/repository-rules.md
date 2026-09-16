---
title: Repository rules
description: What branch protection enforces on main, why merges are squashed, and which tag namespaces are protected.
sidebar:
  order: 7
---

## The main branch

Every repository protects `main` the same way: no direct pushes, no force pushes, no branch deletion, and every change through a pull request whose required checks pass.

Required reviews are set to zero. One developer reviewing his own pull requests is theater, and pretending otherwise would only make the setting lie about who is checking the work. CI is the gatekeeper instead, which is why the list of [required checks](./ci/#what-blocks-a-merge) is as long as it is. If the project gains contributors, this is the first setting to change.

## Squash merges

Pull requests are merged by squash, so `main` is linear and each pull request is one commit. The squash commit takes the pull request's title, which is itself a conventional commit, as its subject and the description as its body.

That is what makes "fix typo" and "respond to review" commits harmless on a feature branch. The discipline applies to the merged history, not to the branch, and work-in-progress commits are expected to be messy.

## Tags

Tags matching `vX.Y.Z` are protected, and only release-please creates them, through the merge-the-release-pull-request mechanism.

Two other namespaces exist outside that protection, both deliberately:

`preview-<pr-number>`, force-updated on every push to a pull request, so that a workflow under review can be called by its own preview tag before anything is released.

The floating `vX` and `vX.Y` tags of the `.github` repository, force-updated onto each new release so that a consumer pinned to `@v2` picks up fixes without editing anything. Both are described in [CI](./ci/#pinning-a-version).
