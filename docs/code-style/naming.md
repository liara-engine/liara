---
title: Naming
description: The C++ naming table, why it follows Unreal rather than the standard library, and how files and namespaces are named.
sidebar:
  order: 3
---

## Identifiers

The table is encoded in the `CheckOptions` block of the C++ clang-tidy baseline. clang-tidy reports deviations as warnings rather than errors, for the reasons in [Static analysis baselines](./checks/#naming-is-reported-not-enforced), so the table is the reference and review is the backstop.

| Identifier kind             | Convention  | Example                      |
|-----------------------------|-------------|------------------------------|
| Class, struct, enum, union  | CamelCase   | `Renderer`, `Transform`      |
| Namespace                   | CamelCase   | `Liara::Graphics`            |
| Function, method            | CamelCase   | `CreateInstance`, `GetSize`  |
| Local variable              | camelBack   | `frameCount`, `vertexBuffer` |
| Function parameter          | camelBack   | `deltaTime`, `windowHandle`  |
| Member variable             | m_CamelCase | `m_Device`, `m_Pipeline`     |
| Static constant             | UPPER_CASE  | `MAX_FRAMES_IN_FLIGHT`       |
| Global constant             | UPPER_CASE  | `DEFAULT_BUFFER_SIZE`        |
| Local constant              | camelBack   | `maxRetries`, `defaultColor` |
| Constant function parameter | camelBack   | `maxRetries`, `defaultColor` |
| Macro                       | UPPER_CASE  | `LIARA_ASSERT`               |
| Enum value                  | CamelCase   | `LightType::Directional`     |
| Template parameter          | CamelCase   | `TComponent`, `TView`        |
| Type alias, typedef         | CamelCase   | `EntityHandle`, `MeshPtr`    |

This follows the Unreal Engine convention, CamelCase for types and methods with an `m_` prefix on members, and it is what the previous engine used. Choosing it over Google style or the standard library's own has a practical reason beyond continuity: Vulkan-Hpp uses camelCase, and having the engine's code visually distinct from it removes a mental context switch on every line that mixes the two.

## Files

A source file takes the name of its primary type. The exception is the ABI shim, which takes the name of the module: `renderer.cpp` for the C interface of `liara-renderer`, lowercase, matching the C side it implements.

Headers are `.h`, implementations are `.cpp`, and template implementations are `.tpp`.

```text
Renderer.h      // the Renderer class
Renderer.cpp

core.cpp        // the core's ABI shim

Logger.h        // a template implementation
Logger.tpp
```

## Namespaces

The top-level namespace is `Liara`, and each module nests under its own:

```cpp
namespace Liara::Core     { /* ECS, math, settings */ }
namespace Liara::Graphics { /* the renderer's implementation */ }
```

The namespace is CamelCase like every other type-level name, and it does not have to match the module's ABI namespace, which is lowercase and prefixed. `liara_renderer_*` on the C side and `Liara::Graphics` on the C++ side are the same module seen from its two regimes.
