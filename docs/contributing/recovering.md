---
title: When things go wrong, and coming back
description: Restoring a green main, abandoning a branch that drifted, and the re-entry procedure after weeks away.
sidebar:
  label: Going wrong, coming back
  order: 8
---

## A bad merge lands on `main`

The priority is restoring green, then investigating. A revert beats a forward fix unless the forward fix is trivially small and obviously correct.

The post-mortem is a brief comment on the pull request or a follow-up issue, saying what was missed and how. It is a paragraph, not a ceremony.

## A branch has drifted too far

A feature branch that has not merged within a week and is hard to rebase gets abandoned and rewritten as smaller pieces.

The drift is the signal, not the problem. It says the original scope was wrong, and rewriting is the correction rather than the punishment.

## CI has been red for days

That is a project emergency. The cause gets investigated and one of three things happens: a fix lands immediately, at the highest priority; the breaking change is reverted, when it cannot be fixed quickly; or the flaky test is quarantined with a tracking issue, when the failure is intermittent and the test is fundamentally sound.

Red CI never becomes the new normal. It is the project's heartbeat, and while it is red, work stops.

## Coming back after a pause

The most likely future contributor to Liara is its current developer, returning after weeks or months. This is the procedure for that.

Read the latest commits on the meta repository's `main`, which say what the project was doing when it stopped. Read the [roadmap](../roadmap/) for the current milestone. Run the workspace bootstrap, and if it no longer works, that is the first thing to fix. Run the demo, and if it no longer runs, that is the second. Look at the issue tracker for `priority:high` items that were active when the pause started.

Then pick a small task. Not the most ambitious one, the most concrete one. The goal of the first day back is to merge one pull request, not to plan the next milestone.

The first pull request after a pause is deliberately small. It restores muscle memory, exercises the CI, and produces something visible. The ambitious work happens in the second or third, once the project feels familiar again.

## Pauses are not failures

Repeated from the [roadmap](../roadmap/) because it matters more than any other sentence here. Pausing for weeks or months is a normal mode of operation, and the project does not require continuous activity. Short branches, green CI and concrete milestones are what let it survive a pause cleanly.

The shame of not having touched it in a month is itself one of the biggest reasons solo projects die. Refusing to feel that shame is part of the project's contract with its developer.
