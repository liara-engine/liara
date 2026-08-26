# ADR 0006 — `manifest.json` as the Single Compatibility Source of Truth

- **Status**: Accepted
- **Date**: 2026-08-26
- **Deciders**: Antoine

## Context

Modules version independently, so something has to record which versions of which modules work together. The original plan was a `compatibility.toml` in the meta repository, listing the combinations that had been validated.

It never got written. Before it reached the top of my todo list, `docs-shared` needed something else: the documentation navbar has to know, for a given repository and version, what other versions exist and what they are compatible with. A `compatibility.toml` in the meta repository could not answer that — it would have had to be fetched from another repository at documentation build time, and it would have described the engine rather than each module's own history. So each repository got a `manifest.json` instead, listing its versions with an `abi_compatibility` field per version.

When `compatibility.toml` finally came up, I read the item and realised the manifests already did almost all of it, and did it better: they live next to the thing they describe, they are updated in the same pull request that changes it, and they are already validated in CI against `.release-please-manifest.json`.

## Decision

`manifest.json` is the only compatibility record. There is no `compatibility.toml` and there will not be one. Anything that needs to know what works with what — the navbar, CI resolving which `liara-interfaces` to build against, the composition tool — reads the manifests.

## Alternatives Considered

**`compatibility.toml` in the meta repository**, as originally planned. It had one capability the manifests do not: a list of engine-level meta-versions, saying that Liara vX is ABI vY plus renderer vZ plus core vW. That is genuinely useful for a user who wants a working set rather than a set of compatible pairs.

I rejected it anyway. One file in one repository describing versions released from six others has to be updated by hand, after the fact, by me, every time anything ships — which means it is wrong roughly as often as I am distracted. The manifests are updated in the pull request that causes the change, and CI refuses a release whose version is missing from its own manifest. Adding a second file that says nearly the same thing, less reliably, to gain one feature was not worth it.

**Both files**, with `compatibility.toml` generated from the manifests. This removes the manual maintenance problem and keeps meta-versions. Rejected as premature rather than as wrong: nothing consumes meta-versions yet, and generating a file for a consumer that does not exist is how you end up maintaining a format nobody reads.

**Git tags alone**, with no manifest, deriving compatibility from what was released when. Rejected: it makes compatibility a matter of chronology, which is exactly the assumption that breaks when the contract moves ahead of its consumers — the normal direction of travel here.

## Consequences

Nothing today records that a specific set of module versions has been tested together. Compatibility is expressed pairwise against the ABI, and a user assembling a set gets "each of these is compatible with ABI 0.2" rather than "this set is known to work". The composition tool of v0.1.x has to derive what it can from the pairs.

If meta-versions become genuinely useful, I would rather build something better than a hand-written TOML file — most likely generated from what the release artifacts actually contain, so it describes what was published rather than what someone remembered to write down.

The manifest schema was designed when `liara-interfaces` was the only repository, and it shows: it asks every repository for an `abi_compatibility` list, including ones where that means nothing (the meta repository, `docs-shared`) and the launcher, which has an ABI requirement rather than an ABI compatibility. Reworking the schema to model repository kinds is on the documentation track.

Because CI resolves the interfaces version from the manifests, a manifest that is wrong produces a build that is wrong. The failure is a compiler error against an unexpected header rather than a message saying the manifest is stale.

## Revisit If

- Meta-versions become a real need — in which case the answer is generation from artifacts, not a maintained file.
- The manifests start being edited only to satisfy CI rather than to describe reality, which would mean they have become the thing I rejected `compatibility.toml` for being.
