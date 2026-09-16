---
title: Performance philosophy
description: Designing data flow that does not prohibit being fast, and measuring to find out whether it worked.
sidebar:
  order: 8
---

Performance is treated as a property of design rather than of optimization passes. The principle is not "make it work, then make it fast". It is "design something whose data flow does not prohibit being fast, then measure to find out whether it is fast enough".

Concretely, four things:

- Hot paths are identified at design time rather than discovered in a profiler.
- Allocation patterns are explicit. Code running every frame does not allocate, and code running at startup may allocate as freely as it likes.
- Data layout follows access patterns rather than object-oriented intuition.
- Benchmarks are written alongside a feature rather than retrofitted to it.

What this does not license is micro-optimization at the expense of readability. Cache-friendliness, allocation discipline and not doing unnecessary work account for the overwhelming majority of the gains available here, and SIMD intrinsics and hand-tuned assembly do not appear anywhere in v0.x.
