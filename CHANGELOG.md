# Changelog

## [0.2.1](https://github.com/liara-engine/liara/compare/v0.2.0...v0.2.1) (2026-09-02)


### Bug Fixes

* **main:** use some variables in a silly output to avoid unused warning ([16347b1](https://github.com/liara-engine/liara/commit/16347b119f7f0630883cf01151f629271e3786ca))

## [0.2.0](https://github.com/liara-engine/liara/compare/v0.1.0...v0.2.0) (2026-08-27)


### ⚠ BREAKING CHANGES

* documentation docker moved to docs-shared

### Features

* add Contributor Covenant 3.0 Code of Conduct ([f338c19](https://github.com/liara-engine/liara/commit/f338c19cecec3951fc49413d168e17f864e7fc5c))
* documentation docker moved to docs-shared ([00c5692](https://github.com/liara-engine/liara/commit/00c5692d8fe0bea0d5979d91d72e30a021e44a3b))
* **main:** enhance module loading and add smoke test option ([b22c6e5](https://github.com/liara-engine/liara/commit/b22c6e52d991be51b580211ef47971747ba7faeb))
* **schema:** add module manifest v2 JSON schema definition ([9fe19c1](https://github.com/liara-engine/liara/commit/9fe19c1ac578fdf3fa624730c405a87eb59e0440))


### Bug Fixes

* **build:** colocate module libraries and executables in a shared bin/ output dir ([d8a1e91](https://github.com/liara-engine/liara/commit/d8a1e91e0e2533ad9c3767cf7e4f3af42b979f52))
* **cmake:** enhance liara_launcher module loading and add runtime checks ([4fac2d9](https://github.com/liara-engine/liara/commit/4fac2d958dfcb40fc3da6ba96f2a4b249b47cd75))
* **cmake:** prevent in-source builds and add IPO support checks ([57be176](https://github.com/liara-engine/liara/commit/57be176bed17334534e7fa22bfcb8a82076493de))
* **cmake:** prevent in-source builds and provide user guidance ([c0be191](https://github.com/liara-engine/liara/commit/c0be191c9101a747d6f24193b3caf4e979bff50d))
* **manifest:** change kind from host to infrastructure in module manifest v2 ([e342b0b](https://github.com/liara-engine/liara/commit/e342b0bdbfc24407be8bdd18b5cfec3e6c3d235a))
* **manifest:** update schema URL for module manifest v2 ([56ffb31](https://github.com/liara-engine/liara/commit/56ffb31a15d8bee43bac144781e399f1008c7e28))
* **manifest:** update schema version and restructure versions for module manifest v2 ([300a79c](https://github.com/liara-engine/liara/commit/300a79c336d93090452d73851362602dd26b52ec))
* **schema:** change required property type to object in module manifest v2 schema ([f4e7690](https://github.com/liara-engine/liara/commit/f4e76902ff78106789c1eb1aaef3dd2abc521136))
* **schema:** update version entry object to enforce property structure in module manifest v2 schema ([931e03f](https://github.com/liara-engine/liara/commit/931e03f7bf3383effa43eab97fcb90dd871b82be))
* **schema:** update versions property type to object in module manifest v2 schema ([e7350f3](https://github.com/liara-engine/liara/commit/e7350f33f9505d9c0f809ed4ef133ea867cec99d))
* **workspace:** validate vcpkg configuration files and ensure consistent baselines ([c0d52a4](https://github.com/liara-engine/liara/commit/c0d52a41a7a97eb915c333733f76c027406eb21d))

## [0.1.0](https://github.com/liara-engine/liara/compare/v0.0.3...v0.1.0) (2026-08-01)


### ⚠ BREAKING CHANGES

* **launcher:** drop support of ABI v0.1.x

### Features

* **build:** add CMake template and enhance build layout resolution ([c7a461b](https://github.com/liara-engine/liara/commit/c7a461b443302cb50a42f03b477d1b1668d0f766))
* **cmake:** add CMakePresets.json template for build configurations ([b11c05e](https://github.com/liara-engine/liara/commit/b11c05e5d408706ae7bcdc199e9835178dc8ef3a))
* **cmake:** add support for mold and lld linkers, and sccache and ccache compiler caches ([3d93ade](https://github.com/liara-engine/liara/commit/3d93ade6334ea2c948b725bb31a0f1ea8b5758fd))
* **config:** add experimental dynamic module loading presets for GCC, Clang and msvc ([5de3e37](https://github.com/liara-engine/liara/commit/5de3e3744c09995efb9260be891e5655223e8381))
* **config:** add module loading options for Liara Launcher ([da36070](https://github.com/liara-engine/liara/commit/da36070dd00203f6c35ac9632107f6baf37ff591))
* **config:** define additional version macros for Liara Launcher ([b38b14d](https://github.com/liara-engine/liara/commit/b38b14de06f49a5308441c867208ab695a7a8f53))
* **config:** update CMake presets for improved module loading and versioning ([e42070e](https://github.com/liara-engine/liara/commit/e42070eab02eb1e2e8441b4d25ab224b42925b4c))
* **editorconfig:** add EditorConfig file for consistent coding styles ([f8dab25](https://github.com/liara-engine/liara/commit/f8dab25b92af11448b3214e4f42d7ae74063b6b1))
* **launcher:** add configuration header and update CMakeLists for launcher build ([4bc2729](https://github.com/liara-engine/liara/commit/4bc27296c0483d78cf9288be3362dcaf690458da))
* **launcher:** implement a core and renderer initialization with compatibility checks ([2b8843a](https://github.com/liara-engine/liara/commit/2b8843a95db8eb1383482278cefde84e0bd276a2))
* **main:** implement dynamic module loading, ABI compatibility checks, and move to ABI v0.1.1 ([5fc51ff](https://github.com/liara-engine/liara/commit/5fc51ff13d1f87727aa1cb416f4e03c3788b544a))
* **manifest:** add new version entry for 0.1.0 with ABI compatibility ([3131317](https://github.com/liara-engine/liara/commit/31313178bfc46da79e8be5c9dab618a31dc983e8))
* **module-loading:** implement cross-platform dynamic library loading and error handling ([989e724](https://github.com/liara-engine/liara/commit/989e724898e233330398d98844a98082cc18f1cc))


### Bug Fixes

* **clang-tidy:** update checks and formatting for improved readability ([afb7e82](https://github.com/liara-engine/liara/commit/afb7e82f275660294fd17d693ed2d5f3fe3e3593))
* **cmake:** update error messages to include documentation links for missing targets ([3d81f97](https://github.com/liara-engine/liara/commit/3d81f973e92d5a28fa142aa30334ae4aabec92cc))
* **docs:** remove markdown links from git clone commands in BOOTSTRAP.md ([d374a2d](https://github.com/liara-engine/liara/commit/d374a2ded7d71a62116a49f322c1c925988af23d))
* **schema:** enforce additionalProperties constraint for modules in version schema ([a9b621b](https://github.com/liara-engine/liara/commit/a9b621b8dadcf52db8954af3ac94ca03c2e12042))
* **schema:** remove additionalProperties constraint for versions in version schema ([872733b](https://github.com/liara-engine/liara/commit/872733ba0c91c9c4f2db5ecad98045585cc16927))
* **schema:** remove format constraint for URI references in version schema ([9919b9b](https://github.com/liara-engine/liara/commit/9919b9bc2cbd48a70be136a3099bb14c00f0cec7))
* **scripts:** add cache variables for Windows debug and release presets in CMakePresets.json.template ([be5a22f](https://github.com/liara-engine/liara/commit/be5a22fd2fa2cf7206d7ee69d41154d18e0656f4))


### Code Refactoring

* **launcher:** drop support of ABI v0.1.x ([b4321d0](https://github.com/liara-engine/liara/commit/b4321d0c7f6475618d8b4c68a52b462a7fb0943c))

## [0.0.3](https://github.com/liara-engine/liara/compare/v0.0.2...v0.0.3) (2026-07-16)


### Features

* **ci:** add commitlint and manifest validation workflows ([2e213b5](https://github.com/liara-engine/liara/commit/2e213b59edb2c56b458d3cff714a6155d3695120))


### Bug Fixes

* **docs:** update bootstrap instructions and documentation hosting details ([d3cce51](https://github.com/liara-engine/liara/commit/d3cce5199d4a153994ce2cde48782a727cb1ba05))
* **release:** add include-component-in-tag option to configuration ([27a17bb](https://github.com/liara-engine/liara/commit/27a17bb111c9e221912cc19d79fc6ef10b51cf3c))
* **release:** remove release-as version from release-please configuration ([763f156](https://github.com/liara-engine/liara/commit/763f156923db6a3c290c1314e532eaceb1d2e751))

## [0.0.2](https://github.com/liara-engine/liara/compare/liara-v0.0.1...liara-v0.0.2) (2026-07-13)


### Features

* **ci:** add GitHub Actions workflow for cleaning up GHCR images weekly ([8877bce](https://github.com/liara-engine/liara/commit/8877bce5b76e1e25356740714ecc093b6287544d))
* **ci:** add GitHub Actions workflow for generating and pushing Docker images ([26d0ece](https://github.com/liara-engine/liara/commit/26d0ece55e3673b331317f7bfc21463889906007))
* **ci:** add GitHub Pages deployment workflow ([919fb7b](https://github.com/liara-engine/liara/commit/919fb7b2c0dcdbb594a04684baf68ff1a700cff3))
* **ci:** update Docker image path and add debug output in workflow ([3d6f676](https://github.com/liara-engine/liara/commit/3d6f67619ec2dd49456249b823b1d3f8803d6766))
* **ci:** update permissions for Docker image generation workflow ([664f73f](https://github.com/liara-engine/liara/commit/664f73f76d2a49c545339140af01531a7c09fcdf))
* **ci:** update permissions for Docker image generation workflow ([2d5d4b1](https://github.com/liara-engine/liara/commit/2d5d4b1dc7d86b7081384942564ff74867010e95))
* **docker:** add curl installation to Dockerfile ([bdd0529](https://github.com/liara-engine/liara/commit/bdd05295c727e0d9580617903878d7601766621e))
* **docker:** add Dockerfile for Liara Engine documentation builder ([aeea999](https://github.com/liara-engine/liara/commit/aeea9993d8dd556a468c8e52e573867513846c34))
* **docker:** add GitHub Actions for Docker preview and cleanup ([acbc545](https://github.com/liara-engine/liara/commit/acbc545308407b04b6171eb0584a185abffbf8a7))
* **docker:** add GitHub Actions for docs preview and cleanup ([7305107](https://github.com/liara-engine/liara/commit/7305107daca2c86e910de8c42c40efa8d58aa263))
* **docker:** add mdBook-mermaid support ([7ab1151](https://github.com/liara-engine/liara/commit/7ab115199b50bf9268ec11d44dcaad5e10153934))
* **docker:** add validation for manifest.json and install ajv-cli ([6660559](https://github.com/liara-engine/liara/commit/66605590c90ddfa74195c63023f4071a1f6356bc))
* **docker:** enhance GitHub Actions for pull request handling and dynamic image tagging ([e495673](https://github.com/liara-engine/liara/commit/e49567309fd2b4aada91fc25aae17ef1e4c4a15b))
* **docker:** increment DOCS_SHARED_VERSION to 0.1.1 ([6b9dc62](https://github.com/liara-engine/liara/commit/6b9dc621c9670579f51e7018d52668ef68921dcb))
* **docker:** update DOCS_SHARED_VERSION to 0.0.5 to use navbar V2 ([3c34c7b](https://github.com/liara-engine/liara/commit/3c34c7bd3988700eb43a82ba77f2bbb853aef1ff))
* **docs:** add GitHub Actions workflow for documentation generation ([7dcca8a](https://github.com/liara-engine/liara/commit/7dcca8a27024587018609dd050ab5f4ce5dd50e5))
* **docs:** add initial documentation configuration for Liara Meta Repo ([7ed4bce](https://github.com/liara-engine/liara/commit/7ed4bce4940e7544989507076eb20adb521e4e05))
* **docs:** add JSON Schema validation for manifest and documentation modules ([bf9de80](https://github.com/liara-engine/liara/commit/bf9de80a2e2a2d37eec7104dedc4404e50a52220))
* **docs:** add mdBook-mermaid configuration to book.toml ([c48f72a](https://github.com/liara-engine/liara/commit/c48f72a41ac5412fbce35f7b8ae27b802cecfdd1))
* **docs:** add support for additional directories in documentation output ([b5a6d65](https://github.com/liara-engine/liara/commit/b5a6d6573ef9ef1cf1ebcd7a7660f991224ce528))
* **docs:** move build script in its own file and add resource processors for documentation generation ([05fd5c0](https://github.com/liara-engine/liara/commit/05fd5c0b2f081c10eabc461b34041ee370eca09c))
* **manifest:** add initial manifest configuration for Liara Engine Meta Repo ([79ebe48](https://github.com/liara-engine/liara/commit/79ebe48ef3fa3f3e5d17dde7abf3c89e1a179946))
* **release:** add configuration for release-please automation ([f81b465](https://github.com/liara-engine/liara/commit/f81b465fcf35b1d12c03887848941648a856089f))
* **replacement:** add debug print statements for file processing and replacement patterns ([2aac519](https://github.com/liara-engine/liara/commit/2aac5194deda085b1a7a61a6b59ab8d545354084))
* **schema:** add documentation module schema definition ([83657f0](https://github.com/liara-engine/liara/commit/83657f042bcc4b98f4735182d69a529aae047bdb))
* **schema:** add meta module definition to modules-registry schema ([78d9960](https://github.com/liara-engine/liara/commit/78d9960bfa34d9356a73a5c0642ff178e2644e6c))
* **schema:** add module manifest JSON schema ([62055ca](https://github.com/liara-engine/liara/commit/62055ca97a8f29985168e4120fddac09f64e35d8))
* **schema:** add modules registry schema ([8df8d5c](https://github.com/liara-engine/liara/commit/8df8d5c1edaf86dad8a29b569946eacf814e8925))
* **schema:** add options for only_mdbook and only_doxygen in modules registry schema ([8187629](https://github.com/liara-engine/liara/commit/818762913773c4ccbe7aa2e2df62037f51283381))
* **schema:** add support for unspecified resources in documentation schema ([9d53d3f](https://github.com/liara-engine/liara/commit/9d53d3f8a46e5216c8a526f7b4e5a0e7e7707bbc))
* **schema:** add version configuration schema for module versions ([5373328](https://github.com/liara-engine/liara/commit/5373328ead378594323488a59f364405649dfab5))


### Bug Fixes

* **book:** update LICENSE resource mapping to include copy-name ([b877f38](https://github.com/liara-engine/liara/commit/b877f38979bfe2a11847630041019b989daa5083))
* **book:** update resource mapping from TOOLING.md to LICENSE ([75e89a2](https://github.com/liara-engine/liara/commit/75e89a2b3711aee310b4b5b7505db9f89c285e54))
* **ci:** add checks for schemas directory and version.json file before copying ([9e37cdc](https://github.com/liara-engine/liara/commit/9e37cdc9fd901218e45a87c3ab054a17061dcf07))
* **ci:** add conditional check for non-empty matrix in Docker build step ([074de08](https://github.com/liara-engine/liara/commit/074de080bce2869125a00b3d04a68fec437c1849))
* **ci:** correct action name for uploading pages artifact ([bfee655](https://github.com/liara-engine/liara/commit/bfee655af7d5f575c03a63122dc1b54c1b925183))
* **ci:** correct JSON formatting in Docker workflow script ([4894072](https://github.com/liara-engine/liara/commit/48940722afd899854b5086ba2ef0d4eb0a9a7c14))
* **ci:** enhance Docker workflow to dynamically build matrix from changed Dockerfiles ([87fdbfa](https://github.com/liara-engine/liara/commit/87fdbfabcc0ca0c3f0dfffb67650c548e6b89279))
* **ci:** improve Doxygen output handling in Dockerfile ([9d13021](https://github.com/liara-engine/liara/commit/9d13021c5e7c99ee763e8637511f867d27c44445))
* **ci:** include scripts directory in Docker workflow triggers ([a442b9b](https://github.com/liara-engine/liara/commit/a442b9b6365eef41feaf3a7ae0f6c817d008bfcf))
* **ci:** remove unnecessary runs-on specification in clean GHCR workflow ([b1a9fb7](https://github.com/liara-engine/liara/commit/b1a9fb78afa77c8189e214359db119a32c5a38c1))
* **ci:** simplify steps in clean GHCR workflow configuration ([0a4210a](https://github.com/liara-engine/liara/commit/0a4210acc1ea20e3000ec0c8150bf33204dd772a))
* **ci:** update Docker tag pattern and improve Dockerfile detection logic ([ba1e8ab](https://github.com/liara-engine/liara/commit/ba1e8aba076f16bd6550a4696c0f3e9aa08d2692))
* **ci:** update Docker tag pattern in workflow configuration ([8d95bcc](https://github.com/liara-engine/liara/commit/8d95bcc1dce168bb9397491ca448dc352aa83480))
* **ci:** update DOCS_SHARED_VERSION to 0.0.3 in Dockerfile ([d31bfd4](https://github.com/liara-engine/liara/commit/d31bfd470802f8f505e07fde495a33f35d65241e))
* **ci:** update GitHub Pages actions to latest versions and configure pages step ([81aab5c](https://github.com/liara-engine/liara/commit/81aab5c48d4d508112b53800431fa23c1465b14d))
* **ci:** update image path in clean GHCR workflow to use target directly ([956b1f0](https://github.com/liara-engine/liara/commit/956b1f06034ec8ab5aef773139956274588e653f))
* **docker:** increment DOCS_SHARED_VERSION to 0.0.6.1 and improve markdown file processing in build-docs.sh ([72b1a09](https://github.com/liara-engine/liara/commit/72b1a0918a220c1aa87d80aac3ed731cb296ae18))
* **docker:** increment DOCS_SHARED_VERSION to 0.1.0 and clean up unnecessary files in Dockerfile ([0eb296d](https://github.com/liara-engine/liara/commit/0eb296da2b5b151a5131b92661404b91620a9c4c))
* **docker:** remove fourth number precision on docs-shared tag ([4b2ad9b](https://github.com/liara-engine/liara/commit/4b2ad9bd22d7c9677cdc704fdce4ed3234dda746))
* **docker:** update ajv-cli and ajv versions in Dockerfile; modify schema version in module-manifest.schema.json ([0eb5320](https://github.com/liara-engine/liara/commit/0eb5320009ed6ea7cf77b83c7cfb670152770e92))
* **docker:** update DOCS_SHARED_VERSION to 0.0.4 ([c9da74b](https://github.com/liara-engine/liara/commit/c9da74b1eddbe23938d7147b15dd76153344ea46))
* **docker:** update DOCS_SHARED_VERSION to 0.0.6 and clean up unnecessary files ([4514206](https://github.com/liara-engine/liara/commit/451420663fbdf72716d20014a2b6dfc0285c91ca))
* **docker:** update Doxygen version to 1.17.0 and increment DOCS_SHARED_VERSION to 0.0.6.2 ([0e53977](https://github.com/liara-engine/liara/commit/0e53977487f459d123343da94db9d1671bd7cfc5))
* **docker:** update file paths for scripts in Dockerfile ([c966be2](https://github.com/liara-engine/liara/commit/c966be285208f7f1944c9267131901f28cefa7c3))
* **docker:** update script paths in Dockerfile to reflect new directory structure ([7e489d2](https://github.com/liara-engine/liara/commit/7e489d28bb48144bf162f34bda506bdb42746437))
* **docs:** improve error handling for documentation build process ([3bfb445](https://github.com/liara-engine/liara/commit/3bfb445d1f90bb469dbdc9ff70efb6ca8ad612ce))
* **docs:** rename 'title' property to 'name' in documentation schema ([c3b5081](https://github.com/liara-engine/liara/commit/c3b508191841500cff3398dd0f7362b5dcc6b927))
* **docs:** update links in documentation for consistency and accuracy, use mermaid instead of ascii arts ([f4194e1](https://github.com/liara-engine/liara/commit/f4194e15bfcfb460519298656f67b562de1aaab3))
* **manifest:** correct description formatting in manifest.json ([ee0538c](https://github.com/liara-engine/liara/commit/ee0538cb25b2aa718c5744780d98f750cf69071e))
* **manifest:** update latest version to 0.0.1 and adjust version compatibility ([b13fcb1](https://github.com/liara-engine/liara/commit/b13fcb1723b14c33afbc2047964838a3cbb42877))
* **replacement:** move warning message for missing replacement file to explicit check ([ccfcff8](https://github.com/liara-engine/liara/commit/ccfcff80a8c7fe290dc102254c83478937bf8268))
* **schema:** add schema property to module-manifest.schema.json ([564af77](https://github.com/liara-engine/liara/commit/564af776d0b7ff80ba6a453ec1b7adb369eee652))
* **schema:** enhance description for replacements in documentation-module.schema.json ([1a1cb0c](https://github.com/liara-engine/liara/commit/1a1cb0c43fe75328d8eb8c85d8d26376f8a00297))
* **schema:** rename schema property to $schema in module-manifest.schema.json ([e511965](https://github.com/liara-engine/liara/commit/e5119650c8137fd4e42c6288643bb4aa7684f729))
