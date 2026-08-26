# ADR 0005 — Version Encoding and Compatibility Semantics

- **Status**: Accepted
- **Date**: 2026-08-26
- **Deciders**: Antoine

> Retrospective. The encoding was decided when `liara-interfaces` was created; the compatibility rule was only fully settled later, and this records both.

## Context

Modules version independently and have to agree at their boundary. Two things needed deciding: how a version is written down in the ABI, and what "compatible" means when two of them differ.

The encoding was the easy part. The check happens before anything else does — the host reads a module's version and decides whether to call into it, and under runtime loading that happens right after symbol resolution, when nothing about the module is yet known to work. I did not want the first thing the program does to be parsing a string. Vulkan packs major, minor and patch into a 32-bit word, that pattern is familiar to anyone who has touched Vulkan, and I copied it.

The rule was messier, and the mess is the reason this record exists. `INTERFACES.md` §8.5 said that a 0.0.x version *required* means exact equality. The code in `liara_version_provides` tested the 0.0.x condition on the *provided* version instead. Both are defensible readings and they disagree on two specific pairs: provided 0.0.5 against required 0.1.0, and provided 0.1.0 against required 0.0.5. I found the divergence during a Phase 0 audit, not through a failure, which means it had been there since the function was written.

## Decision

A version is a single 32-bit word, packed as Vulkan packs it. `LIARA_MAKE_VERSION_UNSAFE` builds one at compile time with no range checking; `liara_try_make_version` is the checked form for computed or untrusted input.

Compatibility is one rule, applied in order:

1. Identical versions are `EXACT`.
2. Different major versions are `INCOMPATIBLE`.
3. **If either side is a 0.0.x version, only exact equality is compatible.**
4. Otherwise, a provided minor at or above the required minor is `COMPATIBLE`; below it, `DEGRADED`.

Rule 3 is symmetric, which resolves the divergence by taking the strictest available reading: both disputed pairs come out `INCOMPATIBLE`. It also makes the patch component significant, which it is nowhere else in the scheme.

The rule is written once. `LIARA_CONSTEXPR_FN` expands to `constexpr` in C++ and `static inline` in C, so the same function serves a `static_assert` in the launcher and a runtime check in a C consumer without a second implementation to keep in step.

## Alternatives Considered

**Textual semver, parsed at negotiation time.** Readable in a log without decoding, and directly  comparable to what the Git tags and `manifest.json` contain. I did not want the earliest check in the program to be the one that allocates and parses, and it would add a "malformed version" failure mode next to the "incompatible version" one at a point where telling them apart helps nobody.

**Comparing the packed words as integers.** The bit layout orders correctly, so `provided >= required` gives the right answer for the common case in one instruction. This is what the launcher actually did until the audit, and it worked — which is the problem. It cannot distinguish `DEGRADED` from `INCOMPATIBLE`, it ignores the major boundary entirely, and it gets rule 3 wrong in both directions. The launcher now goes through the rule.

**Treating 0.0.x as an ordinary minor comparison**, dropping rule 3. One fewer special case, and it would let a 0.0.x module satisfy an 0.0.y requirement instead of demanding lockstep. Rejected: a playground version would silently pass for a released one, and the failure would show up as undefined behaviour at a struct layout mismatch rather than as a refusal at load time. During Phase 0, lockstep is what is actually true.

**Applying rule 3 to only one side**, either reading. Both were on the table since both were already written down somewhere. I took the symmetric version because a rule whose direction has to be remembered is a rule that gets applied backwards, and because the strict reading fails safe: the worst case is refusing a combination that would have worked, which I find out immediately.

## Consequences

The negotiation is a few comparisons on a value already in a register. It works in a `static_assert`, in C and in C++, and before any allocator exists.

Version words are unreadable in a debugger or a log. `liara_version_to_string` exists for that and is the only thing that should be decoding them.

Under rule 3, every 0.0.x release breaks anything pinned to a different one. Phase 0 is therefore more rigid than it looks from outside, and that is deliberate rather than an oversight.

`LIARA_MAKE_VERSION_UNSAFE` corrupts neighbouring fields if a component overflows, and the `_UNSAFE` suffix is the whole mitigation. Thin, but its callers are compile-time literals in headers I review, and the checked form exists for anything else.

The ABI pipeline cannot see a change like this one. `liara_version_provides` keeps its signature when its behaviour changes, so the snapshot shows no diff and the breaking change has to be declared by hand in the pull request title and a `BREAKING CHANGE` footer. Nothing catches me forgetting.

## Revisit If

- Everything on both sides of every negotiation is past 1.0.0, at which point rule 3 is dead code and should be deleted rather than left as a branch nothing exercises.
- A fourth component becomes necessary — a build or revision number — which the bit layout has no room for.
- A module needs to express a range rather than a floor: works with 1.2 through 1.4 but not 1.5. The current rule cannot say that.
