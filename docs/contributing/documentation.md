---
title: Documentation changes
description: What counts as documentation here, and why drift from the code is a bug rather than an untidiness.
sidebar:
  order: 7
---

Documentation changes follow the same workflow as code: branch, commit, pull request, review, merge.

## What counts

The prose pages in any repository's `docs/`, which is these pages and their equivalents elsewhere. The Doxygen comments on public symbols, which become the API reference. The tutorials and user guides, arriving with v0.1. And the code comments that explain something non-obvious.

## Discipline

Code added without documentation is incomplete, and the pull request is not mergeable until the documentation is there.

Documentation that has drifted from the code is a defect on the same level as a bug. It gets reported as one, and the fix updates whichever of the two is wrong, which is not always the documentation.

That last point is worth more than it looks. A page describing an intention in the present tense reads exactly like a page describing a property, and the only thing that tells them apart is somebody opening the file the page is about. Every drift found that way was found by reading code rather than by anything failing, which means nothing else was ever going to catch it.
