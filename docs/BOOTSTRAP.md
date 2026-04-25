# Bootstrap

> The practical setup guide for getting Liara to build on a fresh
> machine. This document covers two platforms: **Arch Linux**
> (the primary development platform) and **Windows 11** (the
> validated cross-platform target).
>
> If you want to set up on a different Linux distribution, the
> Arch instructions are a starting point but you will need to
> translate package names. Distribution-specific support is not a
> project goal; community contributions documenting other
> distributions are welcome but not committed to in this document.

---

## 1. Quick Start

If everything works first try, the procedure is:

**On Arch Linux:**

```bash
# 1. Install system dependencies
sudo pacman -S --needed base-devel cmake ninja git vulkan-devel \
    vulkan-validation-layers shaderc clang gcc mold

# 2. Install vcpkg
git clone https://github.com/microsoft/vcpkg.git ~/.vcpkg
~/.vcpkg/bootstrap-vcpkg.sh
echo 'export VCPKG_ROOT="$HOME/.vcpkg"' >> ~/.zshrc
echo 'export PATH="$VCPKG_ROOT:$PATH"' >> ~/.zshrc
source ~/.zshrc

# 3. Clone the meta repository and bootstrap the workspace
git clone https://github.com/liara-engine/liara.git
cd liara
./scripts/setup-workspace.sh

# 4. Configure and build
cd workspace
cmake --preset=linux-release-clang
cmake --build --preset=linux-release-clang

# 5. Run the demo
./build/linux-release-clang/launcher/liara_launcher
```

**On Windows 11:**

```powershell
# 1. Install Visual Studio 2022 with the "Desktop development with
#    C++" workload (manually, from visualstudio.microsoft.com)

# 2. Install the Vulkan SDK from https://vulkan.lunarg.com/

# 3. Install vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")

# 4. Clone the meta repository and bootstrap the workspace
git clone https://github.com/liara-engine/liara.git
cd liara
.\scripts\setup-workspace.ps1

# 5. Configure and build
cd workspace
cmake --preset=windows-release
cmake --build --preset=windows-release

# 6. Run the demo
.\build\windows-release\launcher\Release\liara_launcher.exe
```

If anything fails, the rest of this document explains each step in
detail and lists known issues.

---

## 2. Arch Linux: Detailed Setup

### 2.1 System Packages

The following packages are required:

| Package                       | Purpose                              |
|-------------------------------|--------------------------------------|
| `base-devel`                  | Compilers, make, autotools           |
| `cmake`                       | Build system (3.29+ via current Arch)|
| `ninja`                       | Build executor for CMake             |
| `git`                         | Source control                       |
| `vulkan-devel`                | Vulkan headers and loader            |
| `vulkan-validation-layers`    | Debug-build validation               |
| `shaderc`                     | `glslc` for shader compilation       |
| `clang`                       | Clang toolchain (recommended)        |
| `gcc`                         | GCC toolchain (also tested)          |
| `mold`                        | Fast linker (recommended)            |
| `clang-tools-extra`           | clang-format, clang-tidy             |

Installation:

```bash
sudo pacman -S --needed base-devel cmake ninja git vulkan-devel \
    vulkan-validation-layers shaderc clang gcc mold clang-tools-extra
```

Verify the toolchain versions meet the project's requirements:

```bash
cmake --version    # Must be 3.29 or newer
clang --version    # Must be 18 or newer
gcc --version      # Must be 14 or newer
glslc --version    # Must report a Vulkan SDK version
```

If any version is below the requirement, update via `pacman -Syu`. As
of 2026, Arch's repositories ship versions that meet all
requirements; if a future Arch release lags, the workaround is to
install from the AUR.

### 2.2 Vulkan SDK Components

`vulkan-devel` from the official repos provides the core Vulkan
development files. The optional but recommended components:

```bash
# Already installed if you ran the command in 2.1, listed for clarity
sudo pacman -S vulkan-validation-layers  # Required for debug builds
sudo pacman -S vulkan-tools              # vulkaninfo, vkcube
sudo pacman -S spirv-tools               # SPIR-V utilities
sudo pacman -S renderdoc                 # GPU debugger (optional)
```

Verify Vulkan is functional:

```bash
vulkaninfo --summary
```

The output should list at least one GPU with Vulkan 1.3 support. If
no GPU is listed, the issue is GPU drivers (Mesa for AMD/Intel,
proprietary or NVK for NVIDIA), not Liara — fix the drivers first.

For your specific hardware (Framework Laptop 16 with Ryzen 7 7840HS,
desktop with Radeon RX 6800), the Mesa drivers are the right choice.
The relevant packages are typically already pulled in by
`vulkan-devel` and a working desktop environment.

### 2.3 vcpkg Installation

vcpkg is installed in the user's home directory, not system-wide.
This is the convention for vcpkg and avoids permission issues:

```bash
git clone https://github.com/microsoft/vcpkg.git ~/.vcpkg
~/.vcpkg/bootstrap-vcpkg.sh
```

Add `VCPKG_ROOT` to the environment. For zsh:

```bash
echo 'export VCPKG_ROOT="$HOME/.vcpkg"' >> ~/.zshrc
echo 'export PATH="$VCPKG_ROOT:$PATH"' >> ~/.zshrc
```

For bash, replace `~/.zshrc` with `~/.bashrc`. For fish:

```fish
set -Ux VCPKG_ROOT $HOME/.vcpkg
fish_add_path $VCPKG_ROOT
```

Reload the shell or open a new terminal. Verify:

```bash
vcpkg --version
```

### 2.4 Clone and Bootstrap the Workspace

The meta repository's bootstrap script clones the other repositories
and configures the workspace:

```bash
git clone https://github.com/liara-engine/liara.git
cd liara
./scripts/setup-workspace.sh
```

The script performs:

- Clone of `liara-interfaces`, `liara-core`, `liara-renderer` into
  `workspace/`.
- Generation of a top-level `CMakeLists.txt` that includes each
  module via `add_subdirectory()`.
- Initial vcpkg dependency resolution (this can take 10-30 minutes
  on first run; subsequent runs are cached).
- Generation of `compile_commands.json` for clangd consumers.

If the script fails partway through, it can be re-run. It is
designed to be idempotent: running it twice does not break anything.

### 2.5 Build and Test

The first build:

```bash
cd workspace
cmake --preset=linux-release-clang
cmake --build --preset=linux-release-clang
```

The `linux-release-clang` preset is the recommended default for
development. The other available presets:

- `linux-debug-gcc` — debug build with GCC.
- `linux-debug-clang` — debug build with Clang.
- `linux-release-gcc` — release build with GCC.
- `linux-release-clang` — release build with Clang. Default.

After a successful build, run the test suite:

```bash
ctest --preset=linux-debug-clang --output-on-failure
```

If all tests pass, the setup is complete.

### 2.6 Run the Demo

The launcher built from the meta repository runs the engine's
sample scene:

```bash
./build/linux-release-clang/launcher/liara_launcher
```

The demo opens a window showing the version-appropriate scene. In
v0.1, this is a single triangle. In later versions, the demo grows
with the engine's capabilities.

If the demo opens and renders without Vulkan validation errors, the
environment is fully functional.

### 2.7 Editor Setup

#### CLion

CLion's CMake support handles the workspace directly:

1. Open `workspace/` in CLion (not the meta repo's root).
2. CLion detects the `CMakeLists.txt` and the presets.
3. Select a preset in the configurations dropdown.
4. Build and run from CLion's UI.

CLion picks up `.clang-format` and `.clang-tidy` automatically and
applies them on save (configurable in preferences).

#### Neovim with clangd

The workspace generates `compile_commands.json` at
`workspace/compile_commands.json`. Configure clangd to find it:

In `~/.config/nvim/lua/config/lsp.lua` (or equivalent):

```lua
require('lspconfig').clangd.setup({
    cmd = {
        'clangd',
        '--background-index',
        '--clang-tidy',
        '--header-insertion=iwyu',
        '--completion-style=detailed',
    },
})
```

Open any source file in `workspace/` and clangd should activate
automatically.

For formatting, use a Neovim plugin like `conform.nvim` or `null-ls`
configured to invoke `clang-format` on save. The project's
`.clang-format` is picked up automatically.

#### VSCode

For VSCode users:

1. Install the `clangd` extension (preferred over the `C/C++`
   extension for this project).
2. Open `workspace/` as the workspace folder.
3. clangd activates automatically.
4. Install the `CMake` extension for preset management.

---

## 3. Windows 11: Detailed Setup

### 3.1 Visual Studio 2022

Install Visual Studio 2022 (Community edition is sufficient) from
https://visualstudio.microsoft.com/. During installation, select
the **"Desktop development with C++"** workload, and verify the
following components are checked:

- MSVC v143 — VS 2022 C++ x64/x86 build tools
- C++ CMake tools for Windows
- Windows 11 SDK (latest)
- Git for Windows (or install separately)

The default installation produces a Visual Studio that can build
the project from inside the IDE, plus command-line tools accessible
from the "x64 Native Tools Command Prompt for VS 2022".

### 3.2 Vulkan SDK

Download and install the Vulkan SDK from https://vulkan.lunarg.com/.
Use the latest 1.3.x release.

The installer:

- Installs the SDK to `C:\VulkanSDK\<version>\`.
- Sets the `VULKAN_SDK` environment variable.
- Adds `$VULKAN_SDK\Bin` to `PATH`.

Verify after a fresh PowerShell session:

```powershell
$env:VULKAN_SDK
glslc --version
vulkaninfo --summary
```

If `vulkaninfo` does not list a GPU with Vulkan 1.3, the issue is
GPU drivers. Update from your GPU vendor's website (NVIDIA, AMD,
Intel) before continuing.

### 3.3 CMake and Git

CMake is included with Visual Studio's CMake tools, but a standalone
CMake 3.29+ is recommended for command-line work:

```powershell
winget install Kitware.CMake
```

Git is included with Visual Studio if you selected the option, or
install separately:

```powershell
winget install Git.Git
```

Verify:

```powershell
cmake --version    # Must be 3.29 or newer
git --version
```

### 3.4 vcpkg

Install vcpkg in `C:\vcpkg`:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

Set `VCPKG_ROOT` permanently for the user:

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
[Environment]::SetEnvironmentVariable(
    "PATH",
    [Environment]::GetEnvironmentVariable("PATH", "User") + ";C:\vcpkg",
    "User")
```

Restart PowerShell and verify:

```powershell
vcpkg --version
```

### 3.5 Clone and Bootstrap

```powershell
git clone https://github.com/liara-engine/liara.git
cd liara
.\scripts\setup-workspace.ps1
```

The PowerShell version of the bootstrap script does the same as the
Linux version: clones modules into `workspace\`, generates the
top-level `CMakeLists.txt`, and resolves vcpkg dependencies.

If PowerShell refuses to run the script due to execution policy:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Then re-run.

### 3.6 Build and Test

```powershell
cd workspace
cmake --preset=windows-release
cmake --build --preset=windows-release
```

Available Windows presets:

- `windows-debug` — debug build, single configuration.
- `windows-release` — release build. Default for development.

Run tests:

```powershell
ctest --preset=windows-debug --output-on-failure
```

### 3.7 Run the Demo

```powershell
.\build\windows-release\launcher\Release\liara_launcher.exe
```

The Visual Studio generator places binaries under a configuration
subdirectory (`Release\`), unlike Ninja's flat layout. This is a
quirk of the multi-config generator.

### 3.8 Editor Setup

#### Visual Studio 2022

1. Open the workspace folder via **File → Open → Folder…**, pointing
   at `workspace\`.
2. Visual Studio detects the `CMakeLists.txt` and the presets.
3. Select a preset from the dropdown next to the build/run buttons.
4. Build and run from the IDE.

Visual Studio picks up `.clang-format` and applies it on save
(configurable in Tools → Options → Text Editor → C/C++ → Code Style).

#### CLion

CLion on Windows works the same as on Linux: open `workspace\`,
select a preset, build.

When configuring CLion, set the toolchain to **Visual Studio** so
that CLion uses MSVC and not MinGW.

#### VSCode

Same as Linux: install clangd extension, open `workspace\`, clangd
activates.

For VSCode to find clangd's binary, install LLVM separately (or use
clangd from the Vulkan SDK, which ships one) and configure the
extension's `clangd.path` setting if needed.

---

## 4. Verifying the Setup

After completing the platform-specific setup, run the verification
script provided by the meta repository:

```bash
# Linux
./scripts/verify-environment.sh

# Windows
.\scripts\verify-environment.ps1
```

The script checks:

- All required tools are present and at acceptable versions.
- `VCPKG_ROOT` is set and points to a valid vcpkg installation.
- The Vulkan SDK is functional.
- A trivial CMake project configures successfully.
- The compile commands database is generated (Linux only).

The script's exit code is the source of truth. If it succeeds, the
environment is ready. If it fails, the message indicates what is
missing or misconfigured.

---

## 5. Common Issues and Workarounds

### vcpkg Dependency Build Fails

The first vcpkg run downloads and builds many dependencies, which
can fail for various reasons (network, disk space, transient errors
in dependency build scripts). The standard recovery:

```bash
cd $VCPKG_ROOT  # Or %VCPKG_ROOT% on Windows
vcpkg remove --recurse <failed-package>
```

Then re-run the workspace bootstrap or CMake configure step. vcpkg
caches successfully-built packages, so the second attempt resumes
from where the first left off.

If a specific package consistently fails, check the vcpkg issue
tracker on GitHub.

### Vulkan Validation Layer Errors at Runtime

If the demo crashes or behaves unexpectedly with messages about
validation layers, the validation layers package may be missing or
mismatched with the Vulkan loader version.

On Arch:

```bash
sudo pacman -S vulkan-validation-layers
sudo pacman -Syu  # Make sure all Vulkan packages are at consistent versions
```

On Windows: reinstall the Vulkan SDK.

If validation errors persist on a known-good driver, capture the
exact message and open a bug report; this is often a real bug in
the engine.

### "GLSL compiler not found"

The `glslc` binary is part of `shaderc` on Arch and the Vulkan SDK
on Windows. If the build complains about `glslc` not being found:

```bash
# Arch
which glslc
# Should output something like /usr/bin/glslc

# Windows (PowerShell)
Get-Command glslc
# Should output something in %VULKAN_SDK%\Bin\
```

If the binary is not on PATH, the SDK installation is incomplete.
Re-run the Vulkan SDK installer or `pacman -S shaderc`.

### CLion Does Not Pick Up Presets

CLion's CMake preset support requires the project to be opened at
the level where `CMakePresets.json` lives. For Liara, this is
`workspace/`, not the meta repo's root.

If presets still don't appear, regenerate the workspace:

```bash
./scripts/setup-workspace.sh --force
```

The `--force` flag wipes and recreates the workspace's
`CMakeLists.txt` and `CMakePresets.json`.

### Slow Linker on Linux

The default GNU `ld` is slow for large C++ projects. The CMake
configuration uses `mold` if it is available, but if for some reason
mold is not used, link times can be painful.

Verify mold is being used:

```bash
ninja -t commands | grep -- '-fuse-ld'
```

Should show `-fuse-ld=mold`. If not, ensure mold is installed
(`sudo pacman -S mold`) and that the CMake cache reflects the
linker choice (delete `build/` and reconfigure if needed).

### "Cannot find compile_commands.json"

clangd looks for `compile_commands.json` in the workspace root or
the build directory. The bootstrap script creates a symlink at
`workspace/compile_commands.json` pointing at the active build's
file, but if the build is not yet configured the symlink is broken.

The fix: configure CMake at least once. The symlink updates
automatically.

### MSVC Compile Times

MSVC's compile times for templates and modern C++ can be
significant. Mitigations:

- Use **Ninja** generator on Windows for faster builds (requires
  installing Ninja and using a different preset). The default
  multi-config Visual Studio generator is more user-friendly but
  slower.
- Enable **sccache**: install via `winget install Mozilla.Sccache`
  and the CMake configuration uses it automatically when found.
- For interactive development, prefer `windows-debug` over
  `windows-release`: debug builds are slightly faster to compile
  due to skipped optimizations.

### Antivirus Slowing Builds (Windows)

Windows Defender or third-party antivirus can dramatically slow
builds by scanning every produced object file. Add an exclusion
for the workspace directory:

```powershell
Add-MpPreference -ExclusionPath "$HOME\src\liara\workspace"
```

Adjust the path to match your actual workspace location. This
typically halves or better the build time on Windows.

---

## 6. Optional Tools

These tools improve the development experience but are not required.

### ccache (Linux)

Compile cache that speeds up incremental and recompile-from-scratch
builds:

```bash
sudo pacman -S ccache
```

The CMake configuration detects ccache and uses it automatically.
Configure ccache's max size:

```bash
ccache -M 10G
```

### sccache (Cross-Platform)

Mozilla's sccache, a Rust-based ccache equivalent that works on
Windows. Useful especially on Windows where ccache support is
limited:

```bash
# Linux/macOS
cargo install sccache  # Or use the package manager.

# Windows
winget install Mozilla.Sccache
```

The CMake configuration uses sccache when found (via
`CMAKE_C_COMPILER_LAUNCHER` and friends).

### RenderDoc (Cross-Platform)

A GPU debugger that captures Vulkan frames for inspection:

```bash
# Arch
sudo pacman -S renderdoc

# Windows: download from https://renderdoc.org/
```

When developing rendering code, RenderDoc is invaluable for
inspecting GPU state, viewing intermediate textures, and diagnosing
draw call issues.

### Tracy Profiler (Cross-Platform)

A real-time frame profiler:

```bash
# Arch
yay -S tracy
# Or build from source: https://github.com/wolfpld/tracy
```

Liara's profiling integration with Tracy is planned for v1.x and is
not yet active.

### Git Hooks (Optional)

The project does not enforce client-side git hooks (see `TOOLING.md`
section 8 for the rationale), but a contributor who wants
client-side checks can install the optional hooks shipped in
`scripts/hooks/`:

```bash
./scripts/install-hooks.sh
```

This installs hooks that run `clang-format` and `commitlint` before
commits. The CI does the same checks server-side regardless.

---

## 7. Updating the Setup

Periodically, the project's dependency requirements change: a new
version of Vulkan, a new compiler version, a new external library.
The procedure:

### Updating System Packages

```bash
# Arch
sudo pacman -Syu

# Windows
winget upgrade --all
```

After a major system update, re-run the verification script:

```bash
./scripts/verify-environment.sh
```

### Updating vcpkg

vcpkg's manifest mode pins dependencies via the registry baseline.
To pull newer versions of dependencies, the project's
`vcpkg-configuration.json` baseline is bumped (a project-side
change, not a developer-side change).

To update the vcpkg tool itself:

```bash
cd $VCPKG_ROOT
git pull
./bootstrap-vcpkg.sh   # Or .bat on Windows
```

### Updating the Workspace

When module repositories receive updates, refresh the workspace:

```bash
cd liara
git pull
./scripts/setup-workspace.sh
```

The bootstrap script `git pull`s each module repository.

### Updating the Vulkan SDK (Windows)

Download the new SDK installer and run it. The installer cleanly
upgrades over the existing installation. After upgrading, regenerate
the build:

```powershell
cd workspace
Remove-Item -Recurse -Force build
cmake --preset=windows-release
cmake --build --preset=windows-release
```

A clean reconfigure is needed because shader compilation paths and
SDK includes may have changed.

---

## 8. Uninstalling

If you want to remove Liara and all its development setup:

### Arch Linux

```bash
# Remove the project
rm -rf ~/src/liara  # Or wherever you cloned

# Remove vcpkg (if not used by other projects)
rm -rf ~/.vcpkg
sed -i '/VCPKG_ROOT/d' ~/.zshrc

# System packages can remain; they are not Liara-specific
```

### Windows

```powershell
# Remove the project
Remove-Item -Recurse -Force C:\src\liara  # Or wherever you cloned

# Remove vcpkg (if not used by other projects)
Remove-Item -Recurse -Force C:\vcpkg
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", $null, "User")

# Visual Studio and Vulkan SDK can remain; they are not Liara-specific
```

The system packages (compilers, Vulkan SDK, Visual Studio) are not
removed by these steps; they are not Liara-specific and may be used
by other projects.

---

## 9. Getting Help

If something in this document does not work as described:

- Check **section 5** for common issues.
- Check the **issues** on the meta repository (closed issues
  included; the answer is often there).
- Open a new issue using the **Bug Report** template, including:
  - Platform (Arch version, Windows version)
  - Compiler version
  - The exact command that failed
  - The complete error output
  - The output of `./scripts/verify-environment.sh`

For general questions about how something works (as opposed to
"this is broken"), use **GitHub Discussions** on the meta
repository.

This document is itself maintained as part of the project; if the
instructions are out of date or unclear, that is a documentation
defect and a PR fixing it is welcome.
