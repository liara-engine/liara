# Changelog

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
