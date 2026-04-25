# Contributing

> The day-to-day workflow for contributing to Liara: from "I want to
> change something" to "the change is merged". This document is for
> the project's primary developer (and for any future contributor),
> not just for external newcomers. It is the procedure that turns
> intent into shipped code.

---

## 1. Who This Document Is For

Liara is primarily a personal learning project. The "contributor"
this document addresses is, most often, the project's own developer
returning to the project after a pause. The workflow is therefore
optimized for one specific failure mode: the developer who has not
touched the project in three weeks and needs to re-enter without
losing momentum.

External contributors are welcome, and the workflow is designed to
be navigable for them as well. The project's small scale makes
formal contributor agreements unnecessary; the MIT license covers
the legal side, and the workflow described here covers the practical
side.

A contributor is anyone who:
- Reports a bug.
- Suggests a feature.
- Submits a pull request to any of the repositories.
- Improves documentation.
- Reviews someone else's pull request.

All of these count, and all of them follow the relevant section of
this document.

---

## 2. Reading Order Before Contributing

Before opening a pull request, a contributor should have at least
skimmed:

1. [`ARCHITECTURE.md`](ARCHITECTURE.md) — the philosophy and
   high-level structure.
2. [`MODULES.md`](MODULES.md) — which repository contains what.
3. [`ROADMAP.md`](ROADMAP.md) — the current milestone.
4. This document — the workflow.
5. [`CODE_STYLE.md`](CODE_STYLE.md) — only when actually writing
   code.
6. [`INTERFACES.md`](https://github.com/liara-engine/liara-interfaces/blob/main/INTERFACES.md)
   — only when modifying anything in `liara-interfaces`.

Reading the entire codebase is not necessary. Reading the documents
that govern the area being modified is.

---

## 3. Setting Up

The setup procedure for local development is documented separately
in [`BOOTSTRAP.md`](BOOTSTRAP.md). The summary:

1. Install the system dependencies (Vulkan SDK, CMake 3.29+,
   compilers, vcpkg).
2. Clone the meta repository.
3. Run `./scripts/setup-workspace.sh`.
4. Configure CMake with a preset.

A successful setup ends with a built engine and passing tests. If
any step fails, the contribution starts there: fix the setup, then
return to the actual contribution.

---

## 4. The Branch Model

The project uses **trunk-based development with short-lived feature
branches**. The model has three rules:

- The `main` branch is always green: it builds, its tests pass, and
  it is in a consistent state. Direct pushes to `main` are
  forbidden; everything goes through pull requests.
- A feature branch lives for **at most one week**. If a change is
  not finished within a week, the branch is too big and is split
  into smaller pieces, each of which can be merged independently.
- Long-running branches (`develop`, `release/*`, `next`) do not
  exist. The release process produces tags, not branches.

### Why Short Branches

The "one week maximum" rule is the most important rule in this
section, and it exists to prevent the most common failure mode of
solo development: a feature branch that grows for two months,
diverges from `main`, becomes unmergeable, and is eventually
abandoned along with the entire project.

Short branches force scope discipline. A change that "needs more
time" is usually a change that is conflating multiple concerns. The
discipline of finishing within a week breaks the change into
mergeable pieces.

This rule has a practical exception: the dogfooding phase
(v0.7-v0.9) involves building a real game, which takes longer than
a week. During dogfooding, the branch model is unchanged for engine
work; the game itself is built in a separate workspace and merged
piece by piece.

### Branch Naming

Branches are named `<type>/<short-description>`:

- `feat/ecs-sparse-set` — a new feature.
- `fix/swapchain-recreation` — a bug fix.
- `refactor/asset-handle-types` — a refactor.
- `docs/architecture-revision` — documentation.
- `chore/update-vcpkg-baseline` — routine maintenance.

The type prefix matches the conventional-commit type. The
description is short, lowercase, hyphen-separated. Branch names are
not exposed to users; they exist for the developer's organization.

---

## 5. Anatomy of a Change

A typical change goes through these stages.

### Stage 1: Identify the Need

The change starts from one of:

- A failing test or a bug report.
- A scheduled milestone task from `ROADMAP.md`.
- An idea recorded in an issue.
- A friction point discovered while using the engine.

Frictions discovered while building the dogfood game (v0.7-v0.9 and
beyond) are recorded as issues even when not immediately fixed; they
become the input for future versions.

### Stage 2: Decide the Scope

Before opening a branch, the change is scoped:

- What is the minimum set of code modifications to make the change
  ship?
- Are there any required interface modifications? (If yes, the
  change crosses the `liara-interfaces` boundary and follows the
  process in section 11.)
- Are there any required tests? (Almost always yes.)
- Are there any required documentation updates? (Often.)

A change that requires touching three modules and four files in
`liara-interfaces` is not a single change; it is multiple changes.
Each one becomes its own branch, opened in a coordinated sequence.

### Stage 3: Open a Branch and Code

The contributor creates a branch from `main`, makes the changes,
and runs `./scripts/check.sh` locally before pushing. The script
runs the same checks the CI runs (clang-format, clang-tidy, tests,
build) and fails fast on the same issues.

A branch is pushed early and often. Pushing to a not-yet-PRed
branch does not run CI (see `TOOLING.md` section 5), so there is no
cost to pushing intermediate states.

### Stage 4: Open a Pull Request

When the change is ready (or even slightly before, as a "draft" PR),
a pull request is opened against `main`. The PR description follows
the template in section 7.

Opening a draft PR before the change is fully ready is a good
pattern: the CI starts validating the branch, the developer can
self-review the diff, and the PR becomes the place to track the
change's progress.

### Stage 5: Self-Review

Even for solo development, the developer reviews their own PR
explicitly. The review pass:

- Reads the diff in GitHub's PR view (which is a different
  perspective than reading it in the editor).
- Checks for forgotten TODOs and debug code.
- Verifies that the PR description accurately describes the change.
- Confirms that the tests cover the new code.
- Confirms that documentation is updated where relevant.

This step catches a surprisingly large fraction of bugs and
incomplete changes. It exists because the human writing the code
and the human reading the diff are, for review purposes, different
people.

### Stage 6: Address CI Feedback

If the CI fails, the failure is addressed before merging. CI failures
are not retriable; if a test is flaky, it is fixed (or quarantined
with an issue tracking the fix).

### Stage 7: Merge

When CI is green and self-review is complete, the PR is merged via
**squash and merge**. The squash commit message follows the PR's
title (which is in conventional commit format) and includes the PR
description as the body.

After merging, the branch is deleted.

### Stage 8: Verify

After the merge, the contributor verifies that `main` is still
green. If a flaky test or unexpected interaction breaks it, a
follow-up fix is opened immediately.

---

## 6. Issues

Issues are used to track:

- Bugs to fix.
- Features to consider for future versions.
- Tasks identified during dogfooding.
- Decisions that need to be made.
- Documentation gaps.

Issues are **not** used as a forum for general questions; those go
to GitHub Discussions on the meta repository.

### Issue Templates

Each repository has issue templates in `.github/ISSUE_TEMPLATE/`,
inherited from the organization-level `.github` repository. The
templates are:

- **Bug Report**: for reporting that something does not work as
  expected. Includes platform, build configuration, repro steps,
  and expected vs actual behavior.
- **Feature Request**: for proposing a new capability. Includes
  the use case, the proposed solution, and considered
  alternatives.
- **Documentation**: for reporting unclear, incorrect, or missing
  documentation.
- **Question**: for asking how to do something. Discussions are
  generally preferred for this; the template is provided for
  questions that turn into actionable items.

Bug reports without a clear repro path may be closed with a request
for more information. This is not unfriendly; it is a constraint of
solo maintenance.

### Issue Triage

Issues are triaged with labels:

- `priority:critical` — blocks the current milestone.
- `priority:high` — should be addressed in the current minor
  version.
- `priority:medium` — should be addressed in the current major
  version line.
- `priority:low` — nice to have.
- `status:needs-info` — waiting for the reporter to provide more
  details.
- `status:wontfix` — explicitly out of scope or contradicts
  project goals.
- `status:duplicate` — already tracked elsewhere.
- `area:*` — which subsystem the issue concerns (`area:ecs`,
  `area:rendering`, etc.).

Triage happens opportunistically. There is no scheduled triage
meeting because there is no team.

---

## 7. Pull Request Template

Every PR follows the template in
`.github/PULL_REQUEST_TEMPLATE.md`, inherited from the
organization-level `.github` repository:

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

The "What/Why/How" structure is borrowed from established practice
and is repeated here because it produces self-explanatory PRs that
remain readable months later, when the original context has faded.

---

## 8. Code Review

The project's only reviewer, in solo development mode, is the
contributor themselves. This is a real review, not a checkbox. The
self-review pass described in section 5, stage 5, is the actual
review.

If external contributors join the project:

- The first review is by the project maintainer.
- A maintainer's PRs are reviewed by themselves (the practical
  reality of solo projects), but the review is still real.
- Substantial PRs by new contributors may be discussed in the issue
  before code is written, to avoid wasted effort.

### Review Standards

A PR is approved when:

- The change accomplishes its stated goal.
- The implementation follows the project's conventions.
- Tests cover the change.
- Documentation is updated.
- The code does not introduce architectural issues (couplings,
  layering violations).

A PR is rejected when:

- The change conflicts with stated project goals or non-goals.
- The implementation works but at the cost of architectural
  cleanliness, with no compelling reason.
- The PR mixes unrelated changes.

Most rejections become "request for changes" rather than outright
closes. The contributor revises and the cycle continues.

### Review Tone

Code review is about code, not the contributor. Comments are
specific and actionable: "this could leak in the error path" rather
than "this is wrong". Phrasings like "I would suggest", "consider",
"what about" are preferred to imperatives.

---

## 9. Commit Discipline

The conventions for commit messages are documented in
[`TOOLING.md`](TOOLING.md) section 8. The discipline:

- Every commit message is a conventional commit.
- The subject line is under 72 characters.
- The body, if present, explains why and is wrapped at 72
  characters.
- Breaking changes are signaled with `!` and a `BREAKING CHANGE:`
  footer.

In a squash-merge workflow, the individual commit messages on a
feature branch matter less than the final squash commit message.
The final squash uses the PR title and description, so PRs are
written with the squash in mind.

### "WIP" Commits Are Fine

During development, "WIP", "checkpoint", "ugh, this doesn't work"
commits are acceptable on the feature branch. They get squashed
away at merge time. The discipline is on the merged history, not on
the in-progress history.

---

## 10. Testing Your Changes

### The Local Check Script

`./scripts/check.sh` is the canonical pre-push check. It runs:

- `clang-format --dry-run` on all source files.
- `clang-tidy` on changed files.
- The `[unit]` test suite.
- A debug build of all modules.

The script's exit code is the source of truth: zero means the CI
will probably pass; non-zero means the CI will probably fail.

Running this script before pushing is recommended but not enforced.
The CI is the actual gatekeeper.

### What to Test

A change is tested at the level appropriate to its scope:

- **Bug fixes**: a regression test that fails before the fix and
  passes after. Without a regression test, the bug is likely to
  recur.
- **New features**: tests covering the feature's expected uses and
  obvious edge cases.
- **Refactors**: existing tests should still pass. New tests are
  not required if behavior is unchanged.
- **Documentation**: prose is "tested" by self-review (does it read
  clearly, is it accurate, are links working).

### Manual Testing

For changes that affect rendering output, manual testing is
unavoidable. The expected procedure:

- Run the demo (or sample game, post-v0.6).
- Verify that the change produces the expected visual result.
- Take a screenshot if the result is non-trivial; attach it to the
  PR.
- For interactive changes (input, audio), describe what was tested
  in the PR.

Manual testing is documented in the PR description so that future
self-review (or external review) can reproduce.

---

## 11. Architectural Proposals (ADRs)

Significant architectural decisions are recorded as Architecture
Decision Records (ADRs) in `docs/adr/`. The process:

### When to Write an ADR

An ADR is required when:

- The change introduces a new tool to the project (new dependency,
  new build step, new third-party library beyond the existing
  list).
- The change modifies a principle in `ARCHITECTURE.md`.
- The change reverses an earlier decision documented in another
  ADR.
- The change has consequences for many parts of the project (not
  just one module's internal structure).

An ADR is **not** required for:

- Bug fixes.
- Implementation refactoring within a module.
- Adding a feature within an established subsystem.

When in doubt, write the ADR. ADRs are cheap to produce; missing
context is expensive to recover later.

### ADR Format

ADRs follow the Michael Nygard format:

```markdown
# NNNN. Title (short, present-tense)

Date: YYYY-MM-DD
Status: Proposed / Accepted / Deprecated / Superseded by NNNN

## Context

What is the issue that motivates this decision?

## Decision

What is the change being proposed or accepted?

## Consequences

What becomes easier or harder as a result of this decision?
```

ADRs are numbered sequentially. The first ADR is `0001-...`. ADRs
are write-once: a reversed decision becomes a new ADR that
supersedes the old, rather than editing the old one.

### ADR Workflow

A proposal-stage ADR is opened as a PR with status "Proposed". The
PR's discussion is where the decision is debated. When merged, the
ADR's status is updated to "Accepted" and the decision takes
effect.

The first ADRs are written during Phase 0 to capture the decisions
already made (multi-repo layout, C ABI, hand-written ECS, etc.).

---

## 12. Cross-Repository Changes

Some changes span multiple repositories: a new function in
`liara-interfaces` requires implementations in `liara-core` and
`liara-renderer`, plus consumption in the `liara` launcher.

The procedure:

1. **Open the change in `liara-interfaces` first.** The interface
   change is the contract. It is reviewed and merged on its own.
2. **Tag a release of `liara-interfaces`** (a minor version bump,
   per the rules in `INTERFACES.md`).
3. **Open changes in consumer modules** referencing the new
   interface version. These are independent PRs in their respective
   repositories.
4. **Update the compatibility matrix** in the meta repository when
   the change is widely available.
5. **Update the launcher**, if applicable, in a separate PR.

The procedure is intentionally several steps. Cross-repo changes
are friction; that friction is what discourages frivolous interface
changes.

### Coordinating PRs

When multiple PRs are part of a single coherent change, they
reference each other in the PR description ("Part of: liara/#42").
This makes the cross-repo set discoverable later.

For breaking interface changes (major version bumps), the consumer
PRs are opened simultaneously with the interface PR, but the
interface PR is merged last (after all consumers are ready). This
avoids leaving consumers broken between the interface change and
their adaptation.

---

## 13. Documentation Updates

Documentation updates follow the same workflow as code changes:
branch, commit, PR, review, merge.

### What Counts as Documentation

- The `.md` files in `docs/` of any repository.
- Doxygen comments on public symbols.
- The user-facing mdBook content.
- Code comments that explain non-obvious code.

### Documentation Discipline

Code that is added without documentation is incomplete. The PR is
not mergeable until documentation is in place.

Documentation that drifts from the code (because the code was
changed but the docs were not updated) is a defect on par with a
bug. Doc drift is reported as a bug; the fix is to update either
the docs or the code, whichever is wrong.

---

## 14. Labels and Project Tracking

Each repository uses a consistent label set:

| Label                | Meaning                                       |
|----------------------|-----------------------------------------------|
| `bug`                | Something does not work.                      |
| `enhancement`        | A new feature or improvement.                 |
| `documentation`      | Documentation issue.                          |
| `good-first-issue`   | Suitable for a new contributor.               |
| `help-wanted`        | Open contribution invited.                    |
| `breaking-change`    | This change breaks compatibility.             |
| `dependencies`       | Concerns external dependencies.               |
| `ci`                 | Concerns the CI/CD pipeline.                  |
| `priority:*`         | Priority level (see issue triage).            |
| `status:*`           | Status modifier (needs-info, wontfix, etc.).  |
| `area:*`             | Subsystem area.                               |

Labels are applied to issues and PRs. They are managed at the
organization level (where possible) so that all repositories use
the same set.

A GitHub Projects board may be used at the meta repository level to
track work across repositories. The board is informal; it is a
visual aid, not a contract.

---

## 15. When Things Go Wrong

### A Bad Merge Lands on `main`

If a merged PR breaks `main`, the response priority is to
**restore `main`'s green state**, then investigate. A revert is
preferred over a forward fix, unless the forward fix is trivially
small and obviously correct.

The post-mortem is a brief comment on the PR or a follow-up issue,
explaining what was missed and how. The post-mortem is not a
ceremony; it is a paragraph.

### A Branch Has Drifted Too Far

If a feature branch has not been merged within a week and is hard
to rebase on `main`, the branch is **abandoned and rewritten** as
smaller pieces. The drift itself is a signal that the original
scope was wrong; the rewrite is the correction.

### The CI Has Been Red for Days

Sustained red CI is a project emergency. The cause is investigated
and either:

- A fix lands immediately (highest priority).
- The breaking change is reverted (if it cannot be fixed quickly).
- The flaky test is quarantined with a tracking issue (if the
  failure is intermittent and the test is fundamentally good).

Red CI is never accepted as "the new normal". The CI is the
project's heartbeat; if it is red, work stops until it is green.

---

## 16. After Pausing the Project

The most likely future contributor to Liara is its current
developer, returning after a pause of weeks or months. This section
is the procedure for that case.

### Re-Entry Procedure

1. **Read the latest commits on `main` of the meta repository.**
   What was the project doing when it paused?
2. **Read `ROADMAP.md`.** What is the current milestone?
3. **Run the workspace bootstrap.** Does it still work? If not,
   that is the first issue to fix.
4. **Run the demo.** Does it still run? If not, that is the second
   issue to fix.
5. **Look at the issue tracker.** Are there `priority:high` items
   that were active when the pause began?
6. **Pick a small task** to re-enter. Not the most ambitious; the
   most concrete. The goal of the first day back is to merge one
   PR, not to plan the next sprint.

The first PR after a pause is intentionally small. It restores
muscle memory, exercises the CI, and produces a visible result. The
ambitious work happens in the second or third PR, once the project
feels familiar again.

### Pauses Are Not Failures

This is repeated from `ROADMAP.md` because it is important: pausing
the project for weeks or months is a normal mode of operation. The
project does not require continuous activity. The discipline of
short branches, green CI, and concrete milestones means that the
project survives pauses cleanly.

The shame of "I haven't touched it in a month" is itself one of the
biggest reasons solo projects die. Refusing to feel that shame is
part of the project's contract with its developer.
