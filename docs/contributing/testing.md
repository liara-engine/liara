---
title: Testing a change
description: The local command that runs what CI runs, what each kind of change owes in tests, and the rendering changes no test can cover.
sidebar:
  order: 4
---

## The local command

`./scripts/liara.sh check` runs the same checks CI does, in the order clang-format, build, clang-tidy, tests. The build is third from last because clang-tidy reads the `compile_commands.json` it produces.

`--fix` applies the formatting rather than only reporting it. `--no-format`, `--no-build`, `--no-tidy` and `--no-tests` each skip a step, and `--preset` overrides the preset. The exit code is 0 only when every step that ran succeeded.

Running it before pushing is recommended and not enforced. CI stays the authority, and the point of `check` is to find out in two minutes rather than after a push.

## What a change owes

**A bug fix** owes a regression test that fails before the fix and passes after. Without it the bug is likely to come back, and nothing will notice.

**A feature** owes tests covering its expected uses and its obvious edge cases.

**A refactor** owes nothing new if behavior is unchanged, and the existing tests still have to pass. That is what makes it a refactor.

**Documentation** is tested by reading it: does it read clearly, is it accurate, do the links resolve.

## What no test covers

Rendering output. For anything affecting what appears on screen, manual testing is unavoidable, and the procedure is to run the demo, check the change produces the expected result, take a screenshot when the result is not trivial and attach it to the pull request, and describe what was tested for interactive changes to input or audio.

That description goes in the pull request rather than in a commit message, so that a later self-review can reproduce it. A manual test nobody wrote down is a manual test nobody can repeat.
