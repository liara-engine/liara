# Changelog

## [0.0.2](https://github.com/liara-engine/liara/compare/v0.0.1...v0.0.2) (2026-07-12)


### Features

* **ci:** add GitHub Actions workflow for cleaning up GHCR images weekly ([8877bce](https://github.com/liara-engine/liara/commit/8877bce5b76e1e25356740714ecc093b6287544d))
* **docker:** add GitHub Actions for Docker preview and cleanup ([acbc545](https://github.com/liara-engine/liara/commit/acbc545308407b04b6171eb0584a185abffbf8a7))
* **docker:** add GitHub Actions for docs preview and cleanup ([7305107](https://github.com/liara-engine/liara/commit/7305107daca2c86e910de8c42c40efa8d58aa263))
* **docker:** add mdBook-mermaid support ([7ab1151](https://github.com/liara-engine/liara/commit/7ab115199b50bf9268ec11d44dcaad5e10153934))
* **docker:** enhance GitHub Actions for pull request handling and dynamic image tagging ([e495673](https://github.com/liara-engine/liara/commit/e49567309fd2b4aada91fc25aae17ef1e4c4a15b))
* **docker:** increment DOCS_SHARED_VERSION to 0.1.1 ([6b9dc62](https://github.com/liara-engine/liara/commit/6b9dc621c9670579f51e7018d52668ef68921dcb))
* **docs:** add mdBook-mermaid configuration to book.toml ([c48f72a](https://github.com/liara-engine/liara/commit/c48f72a41ac5412fbce35f7b8ae27b802cecfdd1))
* **release:** add bump options for minor and patch versions in release configuration ([70a4673](https://github.com/liara-engine/liara/commit/70a46730a2a6a4db8e4cc569419722f3d38de5dd))
* **release:** add configuration for release-please automation ([ba17182](https://github.com/liara-engine/liara/commit/ba17182190721d84050ee8d0472f00637e190339))
* **release:** specify release version as 0.1.0 in configuration ([783767b](https://github.com/liara-engine/liara/commit/783767b5c9967a1a68e8faf714b4b00f299557f8))


### Bug Fixes

* **ci:** remove unnecessary runs-on specification in clean GHCR workflow ([b1a9fb7](https://github.com/liara-engine/liara/commit/b1a9fb78afa77c8189e214359db119a32c5a38c1))
* **ci:** simplify steps in clean GHCR workflow configuration ([0a4210a](https://github.com/liara-engine/liara/commit/0a4210acc1ea20e3000ec0c8150bf33204dd772a))
* **ci:** update image path in clean GHCR workflow to use target directly ([956b1f0](https://github.com/liara-engine/liara/commit/956b1f06034ec8ab5aef773139956274588e653f))
* **docker:** increment DOCS_SHARED_VERSION to 0.0.6.1 and improve markdown file processing in build-docs.sh ([72b1a09](https://github.com/liara-engine/liara/commit/72b1a0918a220c1aa87d80aac3ed731cb296ae18))
* **docker:** increment DOCS_SHARED_VERSION to 0.1.0 and clean up unnecessary files in Dockerfile ([0eb296d](https://github.com/liara-engine/liara/commit/0eb296da2b5b151a5131b92661404b91620a9c4c))
* **docker:** remove fourth number precision on docs-shared tag ([4b2ad9b](https://github.com/liara-engine/liara/commit/4b2ad9bd22d7c9677cdc704fdce4ed3234dda746))
* **docker:** update Doxygen version to 1.17.0 and increment DOCS_SHARED_VERSION to 0.0.6.2 ([0e53977](https://github.com/liara-engine/liara/commit/0e53977487f459d123343da94db9d1671bd7cfc5))
* **docker:** update script paths in Dockerfile to reflect new directory structure ([7e489d2](https://github.com/liara-engine/liara/commit/7e489d28bb48144bf162f34bda506bdb42746437))
* **docs:** update links in documentation for consistency and accuracy, use mermaid instead of ascii arts ([f4194e1](https://github.com/liara-engine/liara/commit/f4194e15bfcfb460519298656f67b562de1aaab3))
* **release:** remove separate-pull-requests configuration to try to fix the race condition ([9368af6](https://github.com/liara-engine/liara/commit/9368af685409a496d3dc801f3ed3c57291b2fb60))
* **release:** update schemas version to 0.0.0 in release-please manifest ([b22b9c4](https://github.com/liara-engine/liara/commit/b22b9c471809cc9001b662b7aaa96e857dd8f2ec))
