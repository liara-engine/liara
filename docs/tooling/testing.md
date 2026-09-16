---
title: Testing
description: doctest and CTest, the label taxonomy and why selection works by exclusion, where a test lives, and the state of coverage.
sidebar:
  order: 4
---

## doctest, under CTest

The project uses doctest. The choice over Catch2 came down to compile time: doctest is roughly an order of magnitude faster to compile, and that difference compounds across every leg of every CI run.

Tests are separate executables per module, in that module's `tests/` directory, each a doctest runner registered with CTest. Targets are named after what they exercise and the language they exercise it from, since the same contract is tested from more than one:

```cmake title="liara-core/tests/CMakeLists.txt"
add_executable(test_core_functions_cxx test_core_functions.cpp)
target_link_libraries(test_core_functions_cxx PRIVATE Liara::Core doctest::doctest)

add_test(NAME test_core_functions_cxx COMMAND test_core_functions_cxx)
set_tests_properties(test_core_functions_cxx PROPERTIES LABELS "unit")
```

A module is linked through its `Liara::` alias and never through the bare target name, so that a compiles against exactly what an external consumer would get. Because everything is registered with CTest, one `ctest` invocation runs every test across every module, locally and in CI alike.

## Categories

Tests are classified with CTest labels rather than doctest tags, because the taxonomy has to cover tests that are not doctest executables at all: the C-only compilation test, the Zig and Rust cross-language tests, the launcher smoke test.

| Label         | What it marks                                                            |
|---------------|--------------------------------------------------------------------------|
| `unit`        | No I/O, no Vulkan, no OS interaction                                     |
| `integration` | Several modules together, may touch the file system or load real assets  |
| `gpu`         | Needs a Vulkan device, so nothing on GitHub-hosted runners can run it    |
| `benchmark`   | Performance measurements, recorded but never gating                      |
| `slow`        | More than a few hundred milliseconds                                     |
| `cross-lang`  | Exercises the C API from a language with a C FFI, currently Zig and Rust |

A test may carry several (`LABELS "integration;slow"`). A category is selected with `ctest -L unit` and excluded with `ctest -LE gpu`.

**Selection works by exclusion.** CI and the developer both run everything and subtract what does not apply: `ctest -LE gpu` in CI, `ctest -LE "slow|gpu"` for a faster local loop. A test carrying no label therefore runs everywhere, which is the safe default, because forgetting a label costs time while the opposite convention would let a whole repository report success without executing anything.

The corollary is that a label is only worth adding when something is meant to exclude it. `unit` is the exception, carried anyway, because `ctest -L unit` is the loop reached for while working and it is the one place inclusion is convenient.

## Discipline

A test lives in the module it tests. One that exercises several modules lives in the module that initiated the feature being tested, which keeps it out of a repository that would then need a sibling to run its own suite.

Test files are named `test_*.cpp` and follow the same style as production code. Integration-style tests go through the public C interfaces of `liara-interfaces`, while a module's own unit tests may reach into its C++ implementation.

Tests are written alongside the feature rather than retrofitted, and a pull request that adds a feature without them is not ready to merge.

## Coverage

Coverage is not collected yet. The plan is gcovr on Linux GCC debug builds producing a Cobertura report, OpenCppCoverage producing the equivalent on Windows, and Codecov for history and pull request comments, and none of that is switched on.

The reason is in [CI](./ci/#what-blocks-a-merge): measuring coverage of Phase 0's placeholder implementations would report a number about scaffolding. It arrives in v0.2, alongside the first code worth covering.

When it does, the target is direction rather than a percentage: a pull request should not lower coverage without saying why. That avoids both the hundred-percent cult, which produces tests that test the test framework, and the excuse that the number means nothing.

## Benchmarks

The `benchmark` label runs in CI on every push to `main`, with results kept as artifacts. There is no dashboard and none is planned before v1.x. Until then the benchmarks are a coarse alarm: a doubling shows up in the CI log, and anything subtler does not.

There is no continuous profiling either. Profiles are captured by hand when something looks wrong.
