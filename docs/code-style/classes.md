---
title: Class design
description: Member order, the rule of five or zero, and the four annotations that make an interface say what it means.
sidebar:
  order: 5
---

## Order within a class

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

Public comes first because it is the interface, and private comes last because it is the implementation detail. Reading a class top to bottom gives the information in order of what a reader is most likely to want.

## Five or zero

A class either defines all five special member functions (destructor, copy constructor, copy assignment, move constructor, move assignment) or none of them.

Defining one or two and leaving the rest is a bug pattern rather than a shortcut, because the compiler generates what is missing with semantics that are rarely what was intended.

## `[[nodiscard]]`

Functions whose return value should be used carry it: queries, factory methods, anything fallible.

```cpp
[[nodiscard]] bool IsValid() const;
[[nodiscard]] std::optional<Mesh> LoadMesh(const std::string& path);
```

Functions where discarding the result is fine, meaning logging and anything that exists for its side effect, do not.

## `explicit`

Single-argument constructors are `explicit` unless the implicit conversion is intended and justified.

```cpp
class Buffer
{
public:
    explicit Buffer(size_t size);   // No implicit conversion from size_t.
};
```

Copy and move constructors are the exception, since they are not user-visible conversions in the sense that matters here.

## Deleted and defaulted

A special member that should not exist is `= delete`d rather than made private, so that the error names what is wrong instead of naming an access violation.

```cpp
class NonCopyable
{
public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
```

A special member that should be the compiler's version is `= default`ed explicitly when the class defines any of the others, which is the rule of five stated as code.

```cpp
class Resource
{
public:
    Resource() = default;
    ~Resource() noexcept;            // Custom destructor.
    Resource(Resource&&) = default;  // Default move.
};
```
