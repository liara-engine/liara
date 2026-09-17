---
title: Pull requests, review and commits
description: The template every pull request follows, what a real self-review looks like, and why messy branch commits are fine.
sidebar:
  order: 3
---

## The template

Inherited from the organization's `.github` repository, in `PULL_REQUEST_TEMPLATE.md`.

```markdown
## What

A one-paragraph summary of what this PR changes.

## Why

The reason for the change. If this addresses an issue, link it
("Closes #42") so it is automatically closed on merge.

## How

A brief description of the approach. For non-trivial changes, this
section explains design choices and discarded alternatives. For
trivial changes (typo fixes, renames), this section may be omitted.

## Testing

- [ ] All affected code is covered by new or existing tests.
- [ ] Tests pass locally on Linux.
- [ ] Tests pass locally on Windows (if changes affect platform code).
- [ ] Manual testing of [specific scenarios] performed.

## Checklist

- [ ] Code follows the conventions in CODE_STYLE.md.
- [ ] CHANGELOG entry added (if applicable; release-please usually
      handles this).
- [ ] Documentation updated (Doxygen comments, user docs, ADR if
      architectural).
- [ ] Interface changes (if any) include version bump and update
      `liara-interfaces`.
- [ ] No unrelated changes mixed in.
```

The What, Why and How structure is borrowed from established practice, and it is here because it produces pull requests that still explain themselves months later, when the context that made them obvious has gone.

## Review

In solo mode the reviewer is the contributor. That is a real review rather than a checkbox, and it is the self-review pass of [stage 5](./branches-and-changes/#the-eight-stages).

If external contributors appear, the maintainer reviews the first one, a maintainer's own pull requests are still reviewed by themselves, which is the practical reality of a solo project, and a substantial change by a new contributor is worth discussing in an issue before any code is written.

### What approval means

A pull request is approved when the change does what it says, the implementation follows the project's conventions, tests cover it, the documentation is updated, and it introduces no architectural problem, meaning no coupling and no layering violation.

It is rejected when it conflicts with a stated goal or non-goal, when it works at the cost of architectural cleanliness with no compelling reason, or when it mixes unrelated changes.

Most rejections are a request for changes rather than a close. The contributor revises and the cycle continues.

### Tone

Review is about the code and not about whoever wrote it. Comments are specific and actionable: "this could leak in the error path" rather than "this is wrong". "I would suggest", "consider" and "what about" beat imperatives, and that holds when reviewing your own work too, where the imperative habit is easiest to fall into.

## Commits

The format, the types, the scopes and what makes a change breaking are all in [Commits and releases](../tooling/commits-and-releases/). They matter more than they look: release-please reads them to decide version numbers and write changelogs, so a badly typed commit produces a wrong release rather than an untidy log.

That discipline applies to the merged history and not to the branch. Because merges are squashed, a feature branch can hold "wip", "fix typo" and "respond to review" without any of it reaching `main`. The squash commit takes the pull request's title, which is where the conventional commit actually has to be correct.
