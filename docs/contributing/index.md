---
title: Contributing
description: The workflow that turns "I want to change something" into "the change is merged", written first for the developer coming back after three weeks away.
sidebar:
  label: Overview
  order: 0
---

## Who this is for

Liara is primarily a personal learning project, so the contributor these pages address is most often its own developer returning after a pause. The workflow is optimized for one specific failure mode: someone who has not touched the project in three weeks and needs to re-enter without losing momentum. [Coming back after a pause](./recovering/#coming-back-after-a-pause) is written for exactly that.

External contributors are welcome, and the workflow is meant to be navigable for them too. The project is small enough that no contributor agreement is needed: the MIT license covers the legal side and these pages cover the practical one.

Contributing means reporting a bug, suggesting a feature, opening a pull request against any of the repositories, improving documentation, or reviewing somebody else's pull request. All of them count and all of them follow the relevant page here.

## Before opening a pull request

The [reading order](../) on the guide's front page is the general one. Two things are worth adding for someone about to change code rather than read about it.

[Code style](../code-style/) is worth reading only when actually writing code, and the [interface guide](https://liara-engine.liara-engine-documentation.workers.dev/liara-interfaces/latest/guides/) only when modifying something in `liara-interfaces`, where it is required reading rather than recommended.

Reading the whole codebase is not necessary. Reading the documents that govern the area being modified is.

## Setting up

[Bootstrap](../bootstrap/) has the procedure in full. The three-line version: install the system dependencies, clone the meta repository, run `./scripts/liara.sh setup` or `.\scripts\liara.ps1 setup`.

A successful setup ends with a built engine and passing tests. If a step fails, that is where the contribution starts: fix the setup first, then go back to what you actually came to do.
