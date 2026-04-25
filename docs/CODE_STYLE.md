# Code Style

> The complete code style reference for Liara. Style rules for C
> interfaces are in [`INTERFACES.md`](../liara-interfaces/INTERFACES.md);
> this document covers the **C++ implementation** behind those
> interfaces, plus the build-system rules that apply uniformly.

---

## 1. Philosophy

Code style rules exist to remove decisions that don't matter, so that
mental energy can be spent on decisions that do. Two developers
arguing about brace placement is two developers not solving the
problem the code is supposed to solve.

This document is therefore **prescriptive**, not descriptive. Each
rule has either a clang-format/clang-tidy enforcement (which makes
the rule mechanical) or an explicit explanation (which makes the
rule defensible). Rules without one or the other are removed.

Where this document and a tool's default disagree, this document
wins and the tool is configured accordingly.

---

## 2. Two Style Boundaries

The project uses two distinct style regimes, separated by the C
interface boundary.

**At the C interface boundary**, the rules in
[`INTERFACES.md`](../liara-interfaces/INTERFACES.md) apply: lowercase
underscore-separated identifiers, prefixed by module, with the rules
laid out in that document. This style is non-negotiable and is the
convention of the broader C ecosystem (Vulkan, SDL, libcurl, POSIX).

**Inside the C++ implementation of any module**, this document's
rules apply: PascalCase for types and methods, m_-prefixed members,
and the conventions detailed below. This style is internal to each
module and other modules never see it.

The transition between the two regimes happens at the wrapper layer
where C++ implementation calls or implements the C interface. This
layer typically looks like:

```cpp
// In renderer/src/c_api.cpp - the wrapper exposing the C interface
extern "C" liara_result liara_renderer_create(
    const liara_renderer_create_info_t* create_info,
    const liara_allocator_t* allocator,
    liara_renderer_handle** out_handle)
{
    try {
        auto* renderer = new Liara::Renderer::Renderer(*create_info, allocator);
        *out_handle = reinterpret_cast<liara_renderer_handle*>(renderer);
        return LIARA_RESULT_SUCCESS;
    }
    catch (const std::bad_alloc&) {
        return LIARA_RESULT_OUT_OF_MEMORY;
    }
    catch (const std::exception&) {
        return LIARA_RESULT_INTERNAL_ERROR;
    }
}
```

Wrapper code is the only place where both styles coexist. Wrappers
are kept thin: they translate types, catch exceptions, and forward.
They contain no business logic.

---

## 3. clang-format Configuration

The complete `.clang-format` file lives at the root of every
repository. It is reproduced here with explanatory comments. The
file itself in repositories does not contain the explanations; this
document is the authoritative explanation.

```yaml
---
# Liara Engine - clang-format configuration
# See docs/CODE_STYLE.md for explanations of each section.

BasedOnStyle: LLVM
Language: Cpp
Standard: c++20

# === INDENTATION & SPACING ===
IndentWidth: 4
TabWidth: 4
UseTab: Never
ContinuationIndentWidth: 4
ConstructorInitializerIndentWidth: 4
IndentCaseLabels: true
IndentPPDirectives: BeforeHash
AccessModifierOffset: -4

# === LINE LENGTH ===
ColumnLimit: 120
ReflowComments: Always
AlignTrailingComments: true
SpacesBeforeTrailingComments: 2

# === BRACES ===
BreakBeforeBraces: Custom
BraceWrapping:
  AfterCaseLabel:        false
  AfterClass:            true
  AfterControlStatement: Never
  AfterEnum:             true
  AfterFunction:         false
  AfterNamespace:        true
  AfterStruct:           true
  AfterUnion:            true
  AfterExternBlock:      false
  BeforeCatch:           true
  BeforeElse:            true
  BeforeWhile:           true
  IndentBraces:          false
  SplitEmptyFunction:    false
  SplitEmptyRecord:      true
  SplitEmptyNamespace:   true

# === SPACING ===
SpaceAfterCStyleCast:               false
SpaceAfterLogicalNot:               false
SpaceAfterTemplateKeyword:          true
SpaceBeforeAssignmentOperators:     true
SpaceBeforeCpp11BracedList:         false
SpaceBeforeCtorInitializerColon:    true
SpaceBeforeInheritanceColon:        true
SpaceBeforeParens:                  ControlStatements
SpaceBeforeRangeBasedForLoopColon:  true
SpaceInEmptyParentheses:            false
SpacesInAngles:                     false
SpacesInContainerLiterals:          false
SpacesInCStyleCastParentheses:      false
SpacesInParentheses:                false
SpacesInSquareBrackets:             false

# === POINTERS & REFERENCES ===
PointerAlignment: Left      # Type* var
ReferenceAlignment: Left    # Type& var

# === INCLUDES ===
SortIncludes: CaseInsensitive
IncludeBlocks: Regroup
IncludeCategories:
  # 0. Special headers that must come first
  - Regex: '^<SDL\.h>$'
    Priority: 0
    SortPriority: 0
  # 1. Module's own header (corresponding .h for a .cpp)
  - Regex: '^"[^/]+\.h"$'
    Priority: 1
    SortPriority: 1
  # 2. Other headers from the same module
  - Regex: '^"[^"]+/[^"]+\.h"$'
    Priority: 2
    SortPriority: 2
  # 3. Liara public interface headers
  - Regex: '^<liara/.*>$'
    Priority: 3
    SortPriority: 3
  # 4. Vulkan, SDL, ImGui, GLM and other third-party with C-style includes
  - Regex: '^<(vulkan|SDL|imgui|glm|cgltf)/.*>$'
    Priority: 4
    SortPriority: 4
  # 5. C++ standard library
  - Regex: '^<[a-z_]+>$'
    Priority: 5
    SortPriority: 5
  # 6. C standard library
  - Regex: '^<[a-z_]+\.h>$'
    Priority: 6
    SortPriority: 6
  # 7. Other (catch-all)
  - Regex: '.*'
    Priority: 7
    SortPriority: 7

# === ALIGNMENT ===
AlignAfterOpenBracket: Align
AlignArrayOfStructures: Left
AlignConsecutiveAssignments: false
AlignConsecutiveBitFields: Consecutive
AlignConsecutiveDeclarations: false
AlignConsecutiveMacros: Consecutive
AlignEscapedNewlines: Left
AlignOperands: AlignAfterOperator

# === BREAK / WRAP ===
AlwaysBreakAfterReturnType: None
AlwaysBreakBeforeMultilineStrings: false
AlwaysBreakTemplateDeclarations: Yes
BinPackArguments: false
BinPackParameters: false
BreakBeforeBinaryOperators: NonAssignment
BreakBeforeConceptDeclarations: Always
BreakBeforeTernaryOperators: true
BreakConstructorInitializers: BeforeComma
BreakInheritanceList: BeforeComma
BreakStringLiterals: true
PackConstructorInitializers: Never

# === LAMBDA ===
AllowShortLambdasOnASingleLine: All
LambdaBodyIndentation: Signature

# === CONTROL FLOW ===
AllowShortBlocksOnASingleLine: Empty
AllowShortCaseLabelsOnASingleLine: false
AllowShortEnumsOnASingleLine: false
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: WithoutElse
AllowShortLoopsOnASingleLine: false

# === EMPTY LINES ===
EmptyLineAfterAccessModifier: Never
EmptyLineBeforeAccessModifier: LogicalBlock
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
SeparateDefinitionBlocks: Always

# === MISC ===
CompactNamespaces: false
FixNamespaceComments: true
NamespaceIndentation: All
SortUsingDeclarations: true
```

### Rationale for the Notable Choices

**`ColumnLimit: 120`** — long enough to avoid frequent wrapping in
modern code (templates and namespaced types are verbose), short
enough to allow side-by-side viewing on a typical monitor.

**`IndentWidth: 4`, `UseTab: Never`** — four-space indentation,
spaces only. Tabs cause inconsistency across editors with different
tab widths. Spaces always render the same.

**Custom braces (Allman-ish for types, K&R for functions/control)** —
classes, structs, enums, and namespaces get their opening brace on
a new line; functions and control statements keep the opening brace
on the same line. This matches the convention used by the previous
engine and is consistent with widely-used styles like Microsoft's.

**`PointerAlignment: Left`, `ReferenceAlignment: Left`** —
`Type* var` and `Type& var`, not `Type *var`. The `*` and `&` are
part of the type, not the variable.

**`IncludeBlocks: Regroup` with explicit categories** — includes are
sorted into well-defined groups, separated by blank lines. This
prevents merge conflicts on includes and makes it obvious where each
header comes from.

**`BinPackArguments: false`, `BinPackParameters: false`** — when
arguments don't fit on one line, each goes on its own line. This is
slightly more verbose but produces clean, readable diffs when an
argument is added or removed.

**`AllowShortFunctionsOnASingleLine: Empty`** — only empty functions
collapse to one line. Single-statement functions get their own
lines, for consistency.

---

## 4. clang-tidy Configuration

The `.clang-tidy` file selects which checks run. Rather than enabling
everything (which produces noise), the configuration enables a
curated set with explicit rationale.

```yaml
---
Checks: >
  bugprone-*,
  -bugprone-easily-swappable-parameters,
  -bugprone-narrowing-conversions,
  cert-*,
  -cert-err58-cpp,
  clang-analyzer-*,
  concurrency-*,
  cppcoreguidelines-*,
  -cppcoreguidelines-avoid-magic-numbers,
  -cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -cppcoreguidelines-pro-type-reinterpret-cast,
  -cppcoreguidelines-pro-type-union-access,
  -cppcoreguidelines-non-private-member-variables-in-classes,
  -cppcoreguidelines-avoid-non-const-global-variables,
  misc-*,
  -misc-non-private-member-variables-in-classes,
  -misc-no-recursion,
  modernize-*,
  -modernize-use-trailing-return-type,
  -modernize-avoid-c-arrays,
  performance-*,
  portability-*,
  readability-*,
  -readability-magic-numbers,
  -readability-identifier-length,
  -readability-named-parameter,

WarningsAsErrors: ''

HeaderFilterRegex: '^(src|include)/.*\.(h|hpp|tpp)$'

CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.StructCase
    value: CamelCase
  - key: readability-identifier-naming.EnumCase
    value: CamelCase
  - key: readability-identifier-naming.UnionCase
    value: CamelCase
  - key: readability-identifier-naming.NamespaceCase
    value: CamelCase
  - key: readability-identifier-naming.FunctionCase
    value: CamelCase
  - key: readability-identifier-naming.MethodCase
    value: CamelCase
  - key: readability-identifier-naming.MemberCase
    value: CamelCase
  - key: readability-identifier-naming.MemberPrefix
    value: 'm_'
  - key: readability-identifier-naming.PrivateMemberCase
    value: CamelCase
  - key: readability-identifier-naming.PrivateMemberPrefix
    value: 'm_'
  - key: readability-identifier-naming.ProtectedMemberCase
    value: CamelCase
  - key: readability-identifier-naming.ProtectedMemberPrefix
    value: 'm_'
  - key: readability-identifier-naming.PublicMemberCase
    value: CamelCase
  - key: readability-identifier-naming.PublicMemberPrefix
    value: 'm_'
  - key: readability-identifier-naming.VariableCase
    value: camelBack
  - key: readability-identifier-naming.ParameterCase
    value: camelBack
  - key: readability-identifier-naming.ConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.GlobalConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.EnumConstantCase
    value: CamelCase
  - key: readability-identifier-naming.StaticConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.MacroDefinitionCase
    value: UPPER_CASE
  - key: readability-identifier-naming.TemplateParameterCase
    value: CamelCase
  - key: readability-identifier-naming.TypeAliasCase
    value: CamelCase
  - key: readability-identifier-naming.TypedefCase
    value: CamelCase
```

### Rationale for Disabled Checks

`bugprone-easily-swappable-parameters` is disabled because it
produces too many false positives in geometric code (e.g., `(x, y,
z)` parameters where the order is inherent to the math).

`bugprone-narrowing-conversions` is disabled because narrowing
conversions are common at the renderer/GPU boundary (uploading
double-precision world coordinates as floats) and adding explicit
casts everywhere produces noise without catching real bugs.

`cppcoreguidelines-avoid-magic-numbers` and the corresponding
readability check are disabled because in graphics code, many
constants are intrinsically meaningful (`vec3(1, 0, 0)` for the X
axis) and naming them adds noise rather than clarity.

`modernize-use-trailing-return-type` is disabled because trailing
return types (`auto foo() -> int`) are a stylistic choice not
universally agreed-on, and consistency with the broader C++ idiom is
preferred.

`modernize-avoid-c-arrays` is disabled because C arrays are required
in some interface contexts (POD structs that cross to C must use C
arrays, not `std::array`).

`readability-named-parameter` is disabled because unnamed parameters
in overrides (where the base class names them) are clearer than
duplicating names.

`cppcoreguidelines-non-private-member-variables-in-classes` and the
matching `misc` check are disabled because POD-style structs with
public members are the right tool for plain data, and the check
doesn't distinguish them from real classes.

---

## 5. C++ Naming Conventions

The naming conventions are enforced by the clang-tidy
`readability-identifier-naming` checks listed above. The summary:

| Identifier kind            | Convention      | Example                  |
|----------------------------|-----------------|--------------------------|
| Class, struct, enum, union | CamelCase       | `Renderer`, `Transform`  |
| Namespace                  | CamelCase       | `Liara::Graphics`        |
| Function, method           | CamelCase       | `CreateInstance`, `GetSize` |
| Local variable             | camelBack       | `frameCount`, `vertexBuffer` |
| Function parameter         | camelBack       | `deltaTime`, `windowHandle` |
| Member variable            | m_CamelCase     | `m_Device`, `m_Pipeline`  |
| Static constant            | UPPER_CASE      | `MAX_FRAMES_IN_FLIGHT`    |
| Global constant            | UPPER_CASE      | `DEFAULT_BUFFER_SIZE`     |
| Macro                      | UPPER_CASE      | `LIARA_ASSERT`            |
| Enum value                 | CamelCase       | `LightType::Directional`  |
| Template parameter         | CamelCase       | `TComponent`, `TView`     |
| Type alias / typedef       | CamelCase       | `EntityHandle`, `MeshPtr` |

This style follows the Unreal Engine convention (PascalCase for
types and methods, m_-prefix for members) and is consistent with the
previous engine's style. The choice over Google-style or
STL-style is deliberate: when working with Vulkan-Hpp (which uses
camelCase), having the engine's own code visually distinct prevents
mental context switches.

### File Names

Source files use the same name as their primary type:

```
Renderer.h
Renderer.cpp
Pipeline.h
Pipeline.cpp
```

Header files use `.h`. Implementation files use `.cpp`. Template
implementation files (when separating template definition from
declaration) use `.tpp` and are included at the bottom of the
corresponding `.h`.

### Namespaces

The top-level namespace is `Liara`. Each module nests under its own
sub-namespace:

```cpp
namespace Liara::Core { /* ECS, math, settings, etc. */ }
namespace Liara::Graphics { /* renderer internals */ }
namespace Liara::Physics { /* (post-v1.0) */ }
```

Within a module, sub-namespaces group related code:

```cpp
namespace Liara::Core::Ecs { /* Entity, Component, World */ }
namespace Liara::Core::Math { /* Vector, Matrix, Quaternion */ }
namespace Liara::Graphics::Vulkan { /* Vulkan-specific helpers */ }
```

The `using namespace` directive is forbidden at file scope and at
namespace scope; it is allowed inside function bodies for short,
local readability gains.

---

## 6. File Organization

### Header Structure

Every header follows this template:

```cpp
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

The order is: file-level docstring, `#pragma once`, includes (in
the configured groups), namespace, type declarations.

### Implementation Structure

Every `.cpp` file starts with the corresponding header's include,
followed by additional includes:

```cpp
#include "Renderer.h"

// === Other includes needed only by the implementation ===
#include <algorithm>

namespace Liara::Graphics
{
    // Implementation here.
}
```

The corresponding header is **always** the first include. This
ensures the header is self-sufficient: it does not silently rely on
includes that happen to come before it elsewhere.

### One Type Per File (Mostly)

Each non-trivial type lives in its own pair of files. Small helper
types (POD structs, simple enums) that exist purely in the service
of a larger type may be co-located in the same file as that type.

Bundling unrelated types in one file is forbidden; each "concept"
gets its own file.

---

## 7. Include Discipline

### Self-Sufficient Headers

Every header includes exactly the headers it needs to be parseable
on its own. A consumer should be able to include any single header
from `include/` without getting compile errors due to missing
includes.

### Include What You Use

Every name used in a file is provided by a header explicitly
included by that file. Indirect inclusion (relying on a
transitively-included header) is forbidden, even when convenient.
This is verified by clangd warnings during development and by a CI
check that strips and re-resolves includes.

### Forward Declarations

When a header only needs to know that a type **exists** (because it
uses pointers or references to it, not the type itself), the
declaration uses a forward declaration rather than including the
type's header:

```cpp
// In Renderer.h
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

The corresponding `.cpp` includes the full headers for `Device` and
`Pipeline`. This pattern reduces compile times and eliminates
spurious recompilations.

### Pragma Once

All headers use `#pragma once`, not include guards. Modern compilers
(MSVC, Clang, GCC) all support `#pragma once` reliably, and it is
shorter and less error-prone than guards. Liara's previous engine
used `#pragma once` and never had a single issue.

---

## 8. Class Design

### Order Within a Class

Members within a class appear in this order:

```cpp
class Foo
{
public:
    // 1. Type aliases and nested types.
    using Handle = uint32_t;

    // 2. Static constants.
    static constexpr size_t MAX_SIZE = 1024;

    // 3. Constructors, destructor, assignment operators.
    explicit Foo(int x);
    ~Foo();
    Foo(const Foo&) = delete;
    Foo& operator=(const Foo&) = delete;
    Foo(Foo&&) noexcept;
    Foo& operator=(Foo&&) noexcept;

    // 4. Public methods, grouped by purpose.
    void DoSomething();
    [[nodiscard]] int GetSomething() const;

protected:
    // Same order as public, applied to protected.

private:
    // 5. Private methods.
    void HelperMethod();

    // 6. Private members.
    int m_X;
    std::unique_ptr<Bar> m_Bar;
};
```

Public comes first because it is the interface; private comes last
because it is the implementation detail. Reading a class top-down
gives the reader the information they need in order of importance.

### Rule of Five (or Zero)

A class either:
- Defines all five special member functions explicitly (destructor,
  copy constructor, copy assignment, move constructor, move
  assignment), or
- Defines none of them (the rule of zero), letting the compiler
  generate them.

Defining one or two but not the rest is a bug pattern (the compiler
generates the rest with surprising semantics).

### `[[nodiscard]]` on Pure Functions

Functions whose return value should be used (queries, factory
methods, fallible operations) are marked `[[nodiscard]]`. The
compiler warns if the result is discarded.

```cpp
[[nodiscard]] bool IsValid() const;
[[nodiscard]] std::optional<Mesh> LoadMesh(const std::string& path);
```

Functions where discarding is acceptable (logging, side-effect-only
operations) are not marked.

### `explicit` Constructors

Single-argument constructors are `explicit` unless implicit
conversion is intentional and well-justified.

```cpp
class Buffer
{
public:
    explicit Buffer(size_t size);   // No implicit conversion from size_t.
};
```

The exception: copy and move constructors are not marked `explicit`
(they are not user-visible conversions in the usual sense).

### Deleted vs Defaulted

Special members that should not be available are explicitly
`= delete`d, not made private:

```cpp
class NonCopyable
{
public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
```

Special members that should be the compiler-generated version are
explicitly `= default`ed when the class also defines one of the
others:

```cpp
class Resource
{
public:
    Resource() = default;
    ~Resource() noexcept;            // Custom destructor.
    Resource(Resource&&) = default;  // Default move.
};
```

---

## 9. Memory Management

### Smart Pointers, Not Raw `new` / `delete`

`new` and `delete` are forbidden at the call site. Memory ownership
is expressed through smart pointers:

- `std::unique_ptr<T>` for unique ownership.
- `std::shared_ptr<T>` for shared ownership (used sparingly).
- Raw `T*` for non-owning references.

Factory functions return `std::unique_ptr<T>` by default; the caller
chooses to convert to `std::shared_ptr<T>` if shared semantics are
needed.

### `std::make_unique` and `std::make_shared`

Smart pointers are constructed with `std::make_unique` and
`std::make_shared`, never with raw `new`:

```cpp
// Good
auto buffer = std::make_unique<Buffer>(size);

// Bad
std::unique_ptr<Buffer> buffer{new Buffer(size)};
```

`make_unique`/`make_shared` are exception-safe and (for shared) more
efficient than constructing the smart pointer from a raw pointer.

### Custom Allocators

For performance-critical paths (per-frame data, ECS storage), custom
allocators are used. The standard library's allocator framework
(`std::allocator_traits`, `std::pmr`) is preferred over hand-rolled
allocators, except where measurement shows the standard interface
adds overhead.

### Raw Pointers Are Non-Owning

A raw pointer in the codebase signals non-ownership. It points to
something that lives elsewhere; it does not need to be deleted.
Functions that take a raw pointer do not take ownership; functions
that return a raw pointer do not transfer ownership.

When a raw pointer can be null, it is documented; when it cannot,
prefer a reference.

---

## 10. Error Handling

The C++ implementation uses exceptions for **exceptional** errors
(out of memory, file not found, GPU error). Returns codes are
**not** used inside the C++ side; they appear only at the C
interface boundary, where exceptions cannot cross.

### Exception Discipline

Exceptions are thrown for:
- Failure to acquire a resource (allocation, file, GPU object).
- Programmer errors detected at runtime (broken invariants).
- Errors from third-party libraries that throw.

Exceptions are **not** thrown for:
- Expected control flow (use `std::optional` or return values).
- Recoverable conditions in the hot path (use a return code).

Each module has a small hierarchy of exception types deriving from
`std::runtime_error` or `std::logic_error`. Custom exception types
include enough context (a message, a code) to be diagnostic.

### `std::expected` for Recoverable Failures

For functions where failure is part of normal operation,
`std::expected<T, E>` is preferred over exceptions:

```cpp
std::expected<Mesh, AssetError> LoadMesh(const std::string& path);
```

The caller pattern-matches on the result. This is more efficient
than exceptions in the common case and makes the failure mode
explicit in the type.

### Assertions

Pre-conditions, post-conditions, and invariants are enforced by
`LIARA_ASSERT` macros that compile to nothing in release builds:

```cpp
void Renderer::DrawMesh(const Mesh& mesh)
{
    LIARA_ASSERT(mesh.IsValid(), "Drawing an invalid mesh");
    // ...
}
```

Assertion failures in debug builds log the location and abort.
Production code does not depend on assertion side effects.

---

## 11. Concurrency

### Threading Primitives

`std::thread`, `std::mutex`, `std::condition_variable`, and
`std::atomic` from the standard library are used. Higher-level
abstractions (`std::async`, `std::future`) are used where they fit;
they are not avoided dogmatically.

### Shared State

Shared mutable state is the exception, not the rule. The architecture
favors message-passing and per-thread state where possible. When
shared state is necessary, it is protected by a mutex held for the
shortest possible time.

### Lock-Free Code

Lock-free code is allowed but discouraged unless measurement shows
it is necessary. The complexity of getting lock-free code correct
exceeds the cost of a `std::mutex` in the common case.

### Thread Safety Documentation

Every public method of a class documents its thread safety:

```cpp
class World
{
public:
    /** @threadsafety Not thread-safe. Caller must serialize calls. */
    Entity CreateEntity();

    /** @threadsafety Thread-safe. Multiple threads may call concurrently. */
    [[nodiscard]] bool IsValid(Entity e) const;
};
```

If thread safety is not documented, the conservative assumption is
"not thread-safe".

---

## 12. Comments

### Doxygen for Public APIs

Every public symbol (class, method, function, type) of a module's
internal C++ API has a Doxygen-style comment:

```cpp
/**
 * @brief Brief summary of what this does.
 *
 * Detailed description, including non-obvious behavior, complexity,
 * or important context.
 *
 * @param x Description of x.
 * @param y Description of y.
 * @return Description of the return value.
 *
 * @pre Pre-conditions.
 * @post Post-conditions.
 * @throws ExceptionType When this is thrown.
 *
 * @threadsafety Thread safety guarantee.
 */
[[nodiscard]] Result DoSomething(int x, int y);
```

Internal helper functions (those in anonymous namespaces or `static`
file-scope) don't require Doxygen, but should still have a comment
if the purpose is not obvious from the name.

### Comments Explain Why, Not What

Code shows what is happening; comments should explain why. A
comment that paraphrases the next line of code is noise:

```cpp
// Bad: comment restates the code.
i++;  // Increment i.

// Good: comment explains intent.
i++;  // Skip the sentinel element at position 0.
```

### TODO Comments

`TODO`, `FIXME`, and `HACK` comments are allowed and encouraged for
known issues. Each comment includes the author's identifier, a date
or version reference, and optionally a tracking issue:

```cpp
// TODO(darkbriks, v0.4): Replace with proper material system.
// FIXME(darkbriks, #42): Race condition under heavy load.
// HACK(darkbriks): Working around Vulkan validation bug.
```

CI scans for `TODO` and `FIXME` and reports counts; growth in either
is visible but not gated.

---

## 13. Templates

### Templates Are Not Banned

Modern C++ uses templates extensively (containers, algorithms,
type-safe APIs). They are not avoided as a matter of style, but
they are kept inside modules; templates do not cross the C interface
boundary.

### Concepts (C++20) for Constraints

Where a template parameter has expectations, those expectations are
expressed with a concept:

```cpp
template <typename T>
concept Component = std::is_trivially_copyable_v<T> && requires
{
    { T::TypeId() } -> std::same_as<TypeId>;
};

template <Component C>
class ComponentStorage
{
    // ...
};
```

Concepts produce better error messages than SFINAE and document the
requirements at the point of use.

### Implementation Files for Templates

Template implementations that are too large for the header live in
`.tpp` files included at the bottom of the corresponding `.h`:

```cpp
// In ComponentStorage.h, at the bottom:
#include "ComponentStorage.tpp"
```

This keeps the header readable while still making the template
visible at the point of instantiation.

---

## 14. C++20 Features

The project uses C++20 freely. Notable features in regular use:

- **Concepts** for template constraints (see above).
- **Ranges** (`std::ranges::sort`, `std::views::filter`) where they
  improve readability.
- **`<format>` and `std::format`** for string formatting.
- **`<span>`** for non-owning array views.
- **`<bit>`** for portable bit manipulation.
- **Designated initializers** for struct construction.
- **`consteval` and `constexpr` improvements**.

C++23 features are not yet used. They will be evaluated when
toolchain support is uniform across all supported compilers
(typically 2-3 years after the standard).

---

## 15. Anti-Patterns

The following are considered anti-patterns and will be flagged in
code review even if clang-tidy does not catch them.

### `using namespace` at Header Scope

Using `using namespace` in a header pollutes the namespace of every
consumer. Forbidden.

### Singleton

The Singleton pattern is forbidden. It hides dependencies, is
hostile to testing, and creates lifetime issues. Where a "single
instance" is needed, an explicit instance is created at startup and
passed to consumers.

### Macros for Logic

Macros are used for include guards (`#pragma once` instead) and for
conditional compilation. They are **not** used to define functions,
constants, or control flow. C++20's `consteval`, `constexpr`, and
templates make function-like macros nearly always avoidable.

### Hungarian Notation

Type prefixes on variable names (`iCount`, `pBuffer`, `szName`) are
forbidden. The type system already tracks the type; the variable
name describes the purpose.

The exception: the `m_` prefix on member variables, which describes
the storage class (member of a class), not the type. This is a
deliberate exception, kept because it provides genuine information
that the type system does not.

### Out Parameters

Out parameters (passing a non-const reference to be filled) are
discouraged in favor of return values, `std::tuple`, or
`std::optional`. They appear in the C interface (because C requires
them) but not in the C++ implementation.

The exception: when filling a large struct that the caller already
owns, an out parameter is acceptable as a performance optimization.

### Global Variables

Mutable global variables are forbidden. Const global constants
(named `UPPER_CASE`, declared `inline constexpr`) are allowed.

### Abbreviations

Identifiers do not abbreviate words: `buffer`, not `buf`; `device`,
not `dev`; `position`, not `pos`. The exceptions are universally
understood (`CPU`, `GPU`, `RGB`, `XYZ`, `id`) and standard math
shortcuts (`x`, `y`, `z`, `i`, `j`).

### Output Operators on Public Types

`operator<<` for `std::ostream` is not defined for public types. If
debug printing is needed, a free function `Debug(const T&)` returns
a string, which is then logged. This avoids dragging `<iostream>`
into headers (it's expensive to compile).

---

## 16. Exceptions to These Rules

These rules are not absolute. A specific situation may justify
deviating from a rule, in which case:

1. The deviation is documented in a comment at the point of
   deviation, explaining why.
2. If the deviation should apply project-wide, it is proposed as a
   change to this document, supported by an ADR.

The goal of having style rules is to remove friction; a rule that
creates more friction than it removes is a wrong rule.
