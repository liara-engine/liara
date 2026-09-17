---
title: Updating and removing
description: Keeping a working setup working when the toolchain moves, and undoing all of it.
sidebar:
  order: 5
---

## System packages

```bash frame="terminal"
# Arch
sudo pacman -Syu

# Windows
winget upgrade --all
```

After a major system update, run the verification again, since that is what tells you which requirement a new version has stopped meeting:

```bash frame="terminal"
./scripts/liara.sh verify --optional      # Linux
.\scripts\liara.ps1 verify --optional     # Windows
```

## vcpkg

The dependency versions come from the registry baseline pinned in `vcpkg-configuration.json`, so pulling newer dependencies is a change to the project rather than something done on a developer machine. Updating the vcpkg tool itself is separate:

```bash frame="terminal"
cd $VCPKG_ROOT
git pull
./bootstrap-vcpkg.sh    # or .bat on Windows
```

## The workspace

```bash frame="terminal"
cd liara
git pull
./scripts/liara.sh setup
```

`setup` pulls each module repository and regenerates the merged files. `--no-pull` skips the fetching and regenerates from what is already cloned, which is what you want while working on a branch.

## The Vulkan SDK on Windows

Run the new installer, which upgrades cleanly over the existing installation, then re-run the configure step so CMake picks up the new location:

```powershell frame="terminal"
.\scripts\liara.ps1 setup --preset windows
```

`windows` is a configure preset. `windows-release` and its siblings are build presets and cannot be passed here.

## Removing everything

```bash frame="terminal"
# Arch
rm -rf ~/src/liara
rm -rf ~/.vcpkg                 # if no other project uses it
sed -i '/VCPKG_ROOT/d' ~/.zshrc
```

```powershell frame="terminal"
# Windows
Remove-Item -Recurse -Force C:\src\liara
Remove-Item -Recurse -Force C:\vcpkg
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", $null, "User")
```

The compilers, the Vulkan SDK and Visual Studio stay. None of them is Liara-specific and other projects are probably using them.
