---
title: Issues and labels
description: What issues track, the four templates, and the label set shared across every repository.
sidebar:
  order: 2
---

Issues track bugs to fix, features to consider for a later version, tasks found while dogfooding, decisions that need making, and documentation gaps.

They are not a forum for general questions. Those go to GitHub Discussions on the meta repository.

## Templates

Each repository inherits its templates from the organization's `.github` repository.

**Bug report**, for something not working as expected. It asks for the platform, the build configuration, the reproduction steps, and what was expected against what happened.

**Feature request**, for a new capability. It asks for the use case, the proposed solution and the alternatives considered.

**Documentation**, for something unclear, incorrect or missing.

**Question**, for how to do something. Discussions are usually better, and the template exists for the questions that turn out to be actionable.

A bug report with no clear reproduction path may be closed asking for more. That is not unfriendliness, it is a constraint of solo maintenance: a bug that cannot be reproduced cannot be fixed, and the request is the fastest route to one that can.

## Triage

Triage happens opportunistically. There is no scheduled triage meeting, because there is no team.

| Label               | Meaning                                               |
|---------------------|-------------------------------------------------------|
| `bug`               | Something does not work                               |
| `enhancement`       | A new feature or an improvement                       |
| `documentation`     | A documentation issue                                 |
| `good-first-issue`  | Suitable for a new contributor                        |
| `help-wanted`       | Open contribution invited                             |
| `breaking-change`   | This breaks compatibility                             |
| `dependencies`      | Concerns an external dependency                       |
| `ci`                | Concerns the pipeline                                 |
| `priority:critical` | Blocks the current milestone                          |
| `priority:high`     | Should be handled in the current milestone            |
| `priority:medium`   | Should be handled eventually                          |
| `priority:low`      | Nice to have                                          |
| `status:needs-info` | Waiting on the reporter                               |
| `status:wontfix`    | Out of scope, or contradicts a project goal           |
| `status:duplicate`  | Already tracked elsewhere                             |
| `area:*`            | Which subsystem, as in `area:ecs` or `area:rendering` |

Labels apply to issues and pull requests alike, and are managed at the organization level where GitHub allows it, so that every repository uses the same set.

A GitHub Projects board at the meta repository may be used to track work across repositories. It is a visual aid rather than a contract, and nothing depends on it being accurate.
