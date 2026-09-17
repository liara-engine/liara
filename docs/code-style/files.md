---
title: File organization and includes
description: What a header looks like, what the shim looks like, and the include discipline that keeps a header parseable on its own.
sidebar:
  order: 4
---

## A header

```cpp title="Renderer.h"
/**
 * @file Renderer.h
 * @brief One-line description of the file's purpose.
 *
 * Optional longer description providing context, design notes, or
 * usage examples.
 */
#pragma once

// === System and standard library includes ===
#include <cstdint>
#include <memory>

// === Third-party includes ===
#include <vulkan/vulkan.hpp>

// === Liara interface includes ===
#include <liara/renderer/renderer.h>

// === Module-internal includes ===
#include "Device.h"
#include "Pipeline.h"

namespace Liara::Graphics
{
    /**
     * @class Renderer
     * @brief Brief description.
     *
     * Detailed description.
     */
    class Renderer
    {
    public:
        // Public interface here.

    private:
        // Private members here.
    };
}
```

The order is the file docstring, `#pragma once`, the includes in their configured groups, the namespace, then the declarations.

## The shim

The ABI shim is the one place in a module where C and C++ meet. It lives in a file named after the module, `src/renderer.cpp` for `liara-renderer`, and it contains nothing but the translation between the two.

```cpp title="src/renderer.cpp"
#include <liara/renderer/renderer.h>

#include "renderer/Renderer.h"

extern "C" {

liara_result_t liara_renderer_create(const liara_renderer_create_info_t* createInfo,
                                     liara_renderer_handle_t* outHandle) {
    if (createInfo == nullptr || outHandle == nullptr) { return LIARA_RESULT_INVALID_ARGUMENT; }

    // No exception crosses this boundary: the C caller has no way to catch one, and unwinding
    // through a foreign frame is undefined. Everything becomes a result code here or nowhere.
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* renderer = new Liara::Graphics::Renderer(*createInfo);
        *outHandle = reinterpret_cast<liara_renderer_handle_t>(renderer);
        return LIARA_RESULT_SUCCESS;
    } catch (const std::bad_alloc&) {
        return LIARA_RESULT_OUT_OF_MEMORY;
    } catch (...) {
        return LIARA_RESULT_UNKNOWN_ERROR;
    }
}

}  // extern "C"
```

Every name crossing the boundary carries the `_t` suffix its declaration gives it. The shim is thin by construction: logic appearing in it belongs on the C++ side, where it can be tested without going through the ABI.

The `catch (...)` is not defensive padding. An exception escaping through a C boundary is undefined behavior, so the shim catches everything, including what it cannot name.

## One type per file, mostly

Each non-trivial type gets its own pair of files. Small helper types, meaning plain-data structs and simple enums existing purely in the service of a larger type, may sit in that type's file.

Bundling unrelated types in one file is not allowed. Each concept gets its own.

## Headers stand alone

Every header includes exactly what it needs to parse on its own. Including any single header from `include/` has to compile, with no missing include to discover.

Every name used in a file comes from a header that file includes explicitly. Relying on a transitive include is not allowed even when it works, because it works until somebody removes an include three files away.

## Forward declarations

When a header only needs to know a type exists, because it uses a pointer or a reference to it rather than the type itself, it forward-declares instead of including.

```cpp title="Renderer.h"
namespace Liara::Graphics
{
    class Device;       // Forward declaration, header not included.
    class Pipeline;

    class Renderer
    {
    public:
        explicit Renderer(Device& device);
    private:
        Device&    m_Device;     // Reference: forward decl is enough.
        Pipeline*  m_Pipeline;   // Pointer: forward decl is enough.
    };
}
```

The matching `.cpp` includes the full headers. The pattern cuts compile times and removes recompilations that had no reason to happen.

## `#pragma once`

Every header uses it, and none uses include guards. MSVC, Clang and GCC all support it reliably, it is shorter, and it cannot be got wrong the way a mistyped guard macro can. The previous engine used it throughout and never had an issue.
