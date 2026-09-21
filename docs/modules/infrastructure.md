---
title: Infrastructure repositories
description: docs-shared, liara-docs and .github. The three repositories that carry no engine code and that every other repository depends on anyway.
sidebar:
  label: Infrastructure
  order: 10
---

Three repositories hold no engine code and never appear in a CMake dependency. They are consumed by GitHub Actions and by the documentation pipeline, which is a dependency that no build failure will ever reveal.

## `docs-shared`

Everything shared by every documentation site in the project, and the image that builds them.

`astro/` is `@liara/starlight-preset`: the Starlight configuration, the design tokens and fonts, the module and version switchers, the reading preferences, and the loader that turns Doxygen XML into Starlight pages. `tools/` holds what runs against the deployed site rather than against a build, meaning asset fingerprinting and garbage collection, the navigation index, the site-wide search index, and version retirement. `hub/` is the landing page at the site root. `modules-registry.json` lists the module identities the switchers are built from.

`Dockerfile` and `build-docs.sh` produce `ghcr.io/liara-engine/liara-documentation-builder`, a Debian image carrying Doxygen and Node plus everything above. CI runs it against each module, which is why a module repository needs no Astro configuration of its own: it provides its prose, its headers and a manifest, and the rest comes from here.

The consequence worth knowing is that everything in this repository is baked into the image. A change to the styles reaches a reader only once the image is rebuilt and the sites are rebuilt with it, which is why one workflow does both. There is no runtime bundle a page fetches and no version pin to keep in step by hand.

The full guide lives in [this repository's own documentation](https://liara-engine.liara-engine-documentation.workers.dev/docs-shared/latest/guides/), and it is worth reading before changing anything here, because the blast radius of a change is every site at once.

## `liara-docs`

The hosting repository. Every published site lives on its `cloudflare-pages` branch under `site/<repo>/<version>/`, written once and never rewritten, which is what keeps a published URL working. `site/_cas/` is the content-addressed store every site's assets are relocated into, so two versions producing identical bytes occupy one path, and `site/pagefind/` is the one search index covering the whole site.

It also holds the Cloudflare Worker that serves all of it, `wrangler.jsonc` and `src/index.js`, and the scheduled sweep that removes stale pull request previews.

## `.github`

The organization-level repository, and the only one whose contents are read by other repositories at run time rather than at build time.

`.github/workflows/reusable-*.yml` holds the pipelines every module calls: build and test, lint, the five ABI checks, the documentation build and deploy and preview, commitlint, manifest validation, image generation, and the housekeeping jobs. `.github/actions/` holds the composite actions those workflows share, chiefly the workspace assembly that build and lint both need. `scripts/` holds the helpers that are not trivial enough to inline in YAML, and a workflow needing one checks this repository out at the ref its caller pinned.

It also carries the issue templates and the pull request template that every repository inherits.

The discipline is the same as `docs-shared`, applied to CI: the logic lives once, and it is versioned by floating tags so that a fix reaches six repositories at once. So does a mistake, which is what the preview tags exist to catch.
