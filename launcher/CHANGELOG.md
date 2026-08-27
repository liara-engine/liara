# Changelog

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
