---
title: Memory management
description: Smart pointers everywhere except one place, and what a raw pointer means when you see one.
sidebar:
  order: 6
---

## Smart pointers, not raw `new` and `delete`

`new` and `delete` do not appear at a call site. Ownership is expressed through the type: `std::unique_ptr<T>` for unique ownership, `std::shared_ptr<T>` for shared ownership and used sparingly, and a raw `T*` for a non-owning reference.

A factory function returns `std::unique_ptr<T>` by default, and the caller converts to `std::shared_ptr<T>` if it needs shared semantics. Doing it the other way round takes the choice away from whoever has to live with it.

Smart pointers are constructed with `std::make_unique` and `std::make_shared`, never from a raw `new`:

```cpp
// Good
auto buffer = std::make_unique<Buffer>(size);

// Bad
std::unique_ptr<Buffer> buffer{new Buffer(size)};
```

Both are exception-safe, and `make_shared` puts the control block and the object in one allocation.

## The one exception

A module's `create` entry point hands back an opaque handle whose lifetime belongs to the caller, released through the matching `destroy`. That is a raw `new` and `delete` pair by construction, because no smart pointer can cross a C interface.

Those two functions, in each module's shim, are the only place raw ownership is allowed. They carry a `// NOLINT(cppcoreguidelines-owning-memory)`, which is the visible marker that this is the exception rather than a slip. Everything behind the handle uses smart pointers as usual.

That is invariant 6 in its concrete form, and [File organization](./files/#the-shim) shows the shim it lives in.

## A raw pointer means non-owning

A raw pointer in the codebase says the thing it points at lives somewhere else and does not need deleting. A function taking one does not take ownership, and a function returning one does not transfer it.

When it can be null, that is documented. When it cannot, a reference says so without needing documentation.

## Custom allocators

Hot paths (per-frame data, ECS storage) use custom allocators. The standard library's framework, `std::allocator_traits` and `std::pmr`, is preferred over anything hand-rolled, except where a measurement shows the standard interface adding overhead.

The condition matters more than the permission. Writing an allocator because the standard one might be slow is how a project acquires a component nobody can debug, and the measurement is what separates that from a real need.
