---
title: "ADR 0011: Removing the DEGRADED compatibility state"
description: Why the fourth compatibility state was removed once a module loader gave it a concrete meaning, and why a partial load is a promise a host cannot keep.
sidebar:
    label: "0011 · Removing DEGRADED"
    order: 11
---

| Status   | Date       | Deciders |
|----------|------------|----------|
| Accepted | 2026-09-20 | Antoine  |

Supersedes rule 4 of [ADR 0005](../0005-version-encoding-and-compatibility/) and nothing else in it.

## Context

[ADR 0005](../0005-version-encoding-and-compatibility/) gave version negotiation four outcomes. Three of them say yes or no. The fourth, `DEGRADED`, said "yes, partly": a provider whose minor version is older than required is usable, minus the entry points it predates.

It was added on a deliberate bet, and the bet is worth stating because it is what this record settles. Adding the state early and dropping it later looked cheaper than discovering a need for it after the enum had shipped. There was no requirement behind it. It was an exercise.

Nothing in the project could produce the state until the module loader arrived. The loader is what gave it a concrete meaning: each module's entry points are listed as data, each entry carries the ABI version that introduced it, and a symbol the module does not export is tolerated and left null exactly when the module predates that entry.

That mechanism was then run against a module written in Rust, standing in for `liara-platform`, with one of its two symbols deliberately not exported. **The launcher loaded it and declared it compatible.**

The immediate cause was arithmetic: every entry was annotated as introduced in ABI 1.0 while the ABI in use was 0.2.4, so every entry counted as newer than every module, and every missing symbol was tolerated. That is a one-line fix.

The design problem underneath it is not. `DEGRADED` hands a host a dispatch table with null members and promises the module is usable. Nothing obliges the host to check a member before calling it, and nothing can: the check would have to appear at every call site, which is precisely what a dispatch table exists to avoid. The failure does not happen at load, where it would name the missing symbol. It happens at the first frame, as a null call.

## Decision

`LIARA_VERSION_COMPAT_DEGRADED` is removed. A provided minor below the required minor is `INCOMPATIBLE`.

A module either exports every entry point its list declares, or it is refused at load, by name. There is no partial state.

The enum value `2` is left as a hole rather than reused, so that a stale consumer reading a `2` finds no meaning rather than a wrong one.

Rules 1 to 3 of ADR 0005, the packed 32-bit encoding, and the 0.0.x lockstep are untouched.

## Alternatives considered

Three, and the first two are what a serious `DEGRADED` would have required.

**The host checks each pointer before use.** Correct, and it is the honest cost of the promise. It was rejected because it puts a branch at exactly the call sites the dispatch table exists to keep clean, and because it makes every call site responsible for a policy decision that belongs to whoever composed the modules.

**The loader fills a missing entry with a generated stub** returning `LIARA_RESULT_NOT_IMPLEMENTED`. Call sites stay clean, and the X-macro lists can generate one stub per entry. Rejected for two reasons: a defensible default return is obvious for `liara_result_t` and arbitrary for everything else, and it converts a refusal the host cannot miss into a runtime error a caller can ignore.

**Keep the state and fix only the arithmetic.** The cheapest option, and the one that preserves the enum as shipped. Rejected because it preserves a state whose only safe user is a host doing the checking of the first alternative. The bet that put `DEGRADED` in the enum said it would be dropped if it cost too much; the loader is the first thing to price it, and this is the price.

## Consequences

The compatibility enum has three reachable states, and `liara_version_provides` never returns the fourth.

A host's dispatch table has no null members after a successful load. That is the property the whole design rests on, and it is now guaranteed by the loader rather than by the caller's vigilance.

Running a module older than the contract stops being possible at all, rather than being possible and unsafe. If the need returns it returns as a requirement with a shape — a shipped plugin, a third-party module on its own cadence — rather than as a mechanism looking for a use.

`liara_abi_oracle` loses a row from its verdict table, and its case for an older minor changes from `DEGRADED` to `INCOMPATIBLE`.

## Revisit if

- A concrete need appears to run a module older than the contract, with someone willing to own the checking that makes it safe. A third-party module on its own release cadence is the likely shape.
- The function lists come to carry enough type information to generate a defensible default return per entry, which would make the stub alternative cheap rather than arbitrary.
- A second host appears and wants a different policy from the launcher's, which would mean the decision belongs to a host rather than to the contract.
