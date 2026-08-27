# Changelog

## [0.1.2](https://github.com/liara-engine/liara/compare/schemas-v0.1.1...schemas-v0.1.2) (2026-08-27)


### Features

* **schema:** add module manifest v2 JSON schema definition ([9fe19c1](https://github.com/liara-engine/liara/commit/9fe19c1ac578fdf3fa624730c405a87eb59e0440))


### Bug Fixes

* **schema:** change required property type to object in module manifest v2 schema ([f4e7690](https://github.com/liara-engine/liara/commit/f4e76902ff78106789c1eb1aaef3dd2abc521136))
* **schema:** update version entry object to enforce property structure in module manifest v2 schema ([931e03f](https://github.com/liara-engine/liara/commit/931e03f7bf3383effa43eab97fcb90dd871b82be))
* **schema:** update versions property type to object in module manifest v2 schema ([e7350f3](https://github.com/liara-engine/liara/commit/e7350f33f9505d9c0f809ed4ef133ea867cec99d))

## [0.1.1](https://github.com/liara-engine/liara/compare/schemas-v0.1.0...schemas-v0.1.1) (2026-07-30)


### Bug Fixes

* **schema:** enforce additionalProperties constraint for modules in version schema ([a9b621b](https://github.com/liara-engine/liara/commit/a9b621b8dadcf52db8954af3ac94ca03c2e12042))
* **schema:** remove additionalProperties constraint for versions in version schema ([872733b](https://github.com/liara-engine/liara/commit/872733ba0c91c9c4f2db5ecad98045585cc16927))
* **schema:** remove format constraint for URI references in version schema ([9919b9b](https://github.com/liara-engine/liara/commit/9919b9bc2cbd48a70be136a3099bb14c00f0cec7))

## 0.1.0 (2026-07-13)


### Features

* **schema:** add documentation module schema definition ([83657f0](https://github.com/liara-engine/liara/commit/83657f042bcc4b98f4735182d69a529aae047bdb))
* **schema:** add meta module definition to modules-registry schema ([78d9960](https://github.com/liara-engine/liara/commit/78d9960bfa34d9356a73a5c0642ff178e2644e6c))
* **schema:** add module manifest JSON schema ([62055ca](https://github.com/liara-engine/liara/commit/62055ca97a8f29985168e4120fddac09f64e35d8))
* **schema:** add modules registry schema ([8df8d5c](https://github.com/liara-engine/liara/commit/8df8d5c1edaf86dad8a29b569946eacf814e8925))
* **schema:** add options for only_mdbook and only_doxygen in modules registry schema ([8187629](https://github.com/liara-engine/liara/commit/818762913773c4ccbe7aa2e2df62037f51283381))
* **schema:** add support for unspecified resources in documentation schema ([9d53d3f](https://github.com/liara-engine/liara/commit/9d53d3f8a46e5216c8a526f7b4e5a0e7e7707bbc))
* **schema:** add version configuration schema for module versions ([5373328](https://github.com/liara-engine/liara/commit/5373328ead378594323488a59f364405649dfab5))


### Bug Fixes

* **docker:** update ajv-cli and ajv versions in Dockerfile; modify schema version in module-manifest.schema.json ([0eb5320](https://github.com/liara-engine/liara/commit/0eb5320009ed6ea7cf77b83c7cfb670152770e92))
* **docs:** rename 'title' property to 'name' in documentation schema ([c3b5081](https://github.com/liara-engine/liara/commit/c3b508191841500cff3398dd0f7362b5dcc6b927))
* **schema:** add schema property to module-manifest.schema.json ([564af77](https://github.com/liara-engine/liara/commit/564af776d0b7ff80ba6a453ec1b7adb369eee652))
* **schema:** enhance description for replacements in documentation-module.schema.json ([1a1cb0c](https://github.com/liara-engine/liara/commit/1a1cb0c43fe75328d8eb8c85d8d26376f8a00297))
* **schema:** rename schema property to $schema in module-manifest.schema.json ([e511965](https://github.com/liara-engine/liara/commit/e5119650c8137fd4e42c6288643bb4aa7684f729))
