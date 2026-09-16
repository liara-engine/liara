---
title: Local workflow
description: The single entry point every local operation goes through, how the workspace is assembled, and what `liara check` runs before you push.
sidebar:
  order: 8
---

## One entry point

Every local operation goes through `./scripts/liara.sh` on Linux and `.\scripts\liara.ps1` on Windows. Both are thin wrappers around `scripts/liara.py`, where the logic actually lives, so there is no platform-specific duplicate drifting out of step with the other.

The subcommands are `verify`, `setup`, `build`, `test`, `launch`, `clean` and `check`.

## Setting up a workspace

1. Clone the meta repository.
2. Run `./scripts/liara.sh verify` to check the environment. `--optional` also checks the tools that are recommended rather than required.
3. Run `./scripts/liara.sh setup`.
4. Open `workspace/` in whatever editor you use.

`setup` does the assembly: it clones every module repository into `workspace/`, generates the superbuild `CMakeLists.txt`, a merged `vcpkg.json` that is the union of every module's dependencies and features, and the workspace `CMakePresets.json`, then configures CMake and, on Linux, symlinks `compile_commands.json` at the workspace root. vcpkg resolves dependencies during that configure step, which can be a 10-to-30-minute wait on a machine that has never built the project.

`--preset` overrides the preset, `--no-configure` stops before CMake, `--no-pull` leaves existing clones alone (which is what you want while working on a branch), and `--ssh` clones over SSH. Re-running `setup` is safe and is the normal way to refresh a workspace.

The Visual Studio generator produces no `compile_commands.json`, so the symlink step is skipped on Windows and clangd-based tooling there needs a Ninja configuration of its own.

## Before pushing

There are no pre-commit hooks, and `./scripts/liara.sh check` is what replaces them: it runs the same checks CI does, in the order clang-format, build, clang-tidy, CTest. The build comes third from last on purpose, because clang-tidy reads the `compile_commands.json` the build produces.

`--fix` applies the formatting instead of only reporting it. `--no-format`, `--no-build`, `--no-tidy` and `--no-tests` each skip a step, and `--preset` overrides the preset. The exit code is 0 only when every step that ran succeeded.

Running it is recommended and not enforced. CI stays the authority, and the point of `check` is to find out in two minutes rather than after a push.

## Editors

Anything that consumes `compile_commands.json` works: CLion opens the workspace `CMakeLists.txt` directly, VSCode through the C/C++ or clangd extension, Neovim through clangd and nvim-lspconfig, Vim through vim-lsp or coc.nvim.

The `.clang-format` and `.clang-tidy` at each repository root drive the editor's formatting and linting, so an editor is configured by the repository rather than by a committed settings file. Nothing editor-specific is committed.
