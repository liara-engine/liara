# Changelog

## [1.0.0](https://github.com/liara-engine/liara/compare/launcher-v0.1.2...launcher-v1.0.0) (2026-09-22)


### ⚠ BREAKING CHANGES

* LIARA_LAUNCHER_MODULE_LOADING is gone. A build directory configured before this change carries a stale cache entry and has to be deleted and reconfigured.

### Features

* framework layer ([#43](https://github.com/liara-engine/liara/issues/43)) ([b9896ca](https://github.com/liara-engine/liara/commit/b9896ca8c6f3c9a457e714342d1f42901a2db787))
* **main:** the main now possess the loop ([248f0fc](https://github.com/liara-engine/liara/commit/248f0fcec3e49ab1a3bc957f75ae36e46207d6c0))
* module loader ([#42](https://github.com/liara-engine/liara/issues/42)) ([73c3179](https://github.com/liara-engine/liara/commit/73c3179ace9e9116d00a88fe217076355476ad82))
* platform introduction ([#40](https://github.com/liara-engine/liara/issues/40)) ([53ee88d](https://github.com/liara-engine/liara/commit/53ee88dc9253352c11329e5e9c6196d046f06e37))
* remove DEGRADED compatibility state and update related documentation ([5cb1031](https://github.com/liara-engine/liara/commit/5cb1031a0b7ab050ff24d15075dd0218e880c114))


### Bug Fixes

* **CMake:** update bootstrap guide link to use lowercase ([e879676](https://github.com/liara-engine/liara/commit/e879676b3a5afa522c8fc58cdc90ac245915a946))

## [0.1.2](https://github.com/liara-engine/liara/compare/launcher-v0.1.1...launcher-v0.1.2) (2026-09-17)


### Bug Fixes

* **main:** use some variables in a silly output to avoid unused warning ([16347b1](https://github.com/liara-engine/liara/commit/16347b119f7f0630883cf01151f629271e3786ca))

## [0.1.1](https://github.com/liara-engine/liara/compare/launcher-v0.1.0...launcher-v0.1.1) (2026-08-27)


### Features

* **main:** enhance module loading and add smoke test option ([b22c6e5](https://github.com/liara-engine/liara/commit/b22c6e52d991be51b580211ef47971747ba7faeb))


### Bug Fixes

* **cmake:** enhance liara_launcher module loading and add runtime checks ([4fac2d9](https://github.com/liara-engine/liara/commit/4fac2d958dfcb40fc3da6ba96f2a4b249b47cd75))
* **cmake:** prevent in-source builds and provide user guidance ([c0be191](https://github.com/liara-engine/liara/commit/c0be191c9101a747d6f24193b3caf4e979bff50d))

## [0.1.0](https://github.com/liara-engine/liara/compare/launcher-v0.0.1...launcher-v0.1.0) (2026-08-01)


### ⚠ BREAKING CHANGES

* **launcher:** drop support of ABI v0.1.x

### Features

* **module-loading:** implement cross-platform dynamic library loading and error handling ([989e724](https://github.com/liara-engine/liara/commit/989e724898e233330398d98844a98082cc18f1cc))


### Code Refactoring

* **launcher:** drop support of ABI v0.1.x ([b4321d0](https://github.com/liara-engine/liara/commit/b4321d0c7f6475618d8b4c68a52b462a7fb0943c))

## 0.0.1 (2026-07-29)


### Features

* **config:** add module loading options for Liara Launcher ([da36070](https://github.com/liara-engine/liara/commit/da36070dd00203f6c35ac9632107f6baf37ff591))
* **config:** define additional version macros for Liara Launcher ([b38b14d](https://github.com/liara-engine/liara/commit/b38b14de06f49a5308441c867208ab695a7a8f53))
* **launcher:** add configuration header and update CMakeLists for launcher build ([4bc2729](https://github.com/liara-engine/liara/commit/4bc27296c0483d78cf9288be3362dcaf690458da))
* **launcher:** implement a core and renderer initialization with compatibility checks ([2b8843a](https://github.com/liara-engine/liara/commit/2b8843a95db8eb1383482278cefde84e0bd276a2))
* **main:** implement dynamic module loading, ABI compatibility checks, and move to ABI v0.1.1 ([5fc51ff](https://github.com/liara-engine/liara/commit/5fc51ff13d1f87727aa1cb416f4e03c3788b544a))


### Bug Fixes

* **cmake:** update error messages to include documentation links for missing targets ([3d81f97](https://github.com/liara-engine/liara/commit/3d81f973e92d5a28fa142aa30334ae4aabec92cc))
