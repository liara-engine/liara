---
title: Optional tools
description: Four tools that improve the experience without being required, and what each one is actually for.
sidebar:
  order: 4
---

None of these is required, and the build does not mention their absence.

## ccache, on Linux

A compile cache, picked up automatically by the workspace `CMakeLists.txt` when it is on `PATH`.

```bash frame="terminal"
sudo pacman -S ccache
ccache -M 10G
```

## sccache, on both

Mozilla's Rust-based equivalent, and the one to use on Windows, where ccache support is thin. The workspace prefers it over ccache when both are present.

```bash frame="terminal"
# Linux
cargo install sccache

# Windows
winget install Mozilla.Sccache
```

## RenderDoc

A GPU debugger that captures Vulkan frames for inspection. When working on rendering code it is what shows you the GPU state, the intermediate textures and what each draw call actually did, which is otherwise guesswork.

```bash frame="terminal"
# Arch
sudo pacman -S renderdoc
```

On Windows, from renderdoc.org.

## Tracy

A real-time frame profiler.

```bash frame="terminal"
yay -S tracy
```

Liara's integration with it is planned for v1.x and does not exist yet, so installing it now buys nothing for this project specifically.
