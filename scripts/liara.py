#!/usr/bin/env python3
import sys
import os
import platform
import subprocess
import shutil
import json
import argparse
import tempfile
from pathlib import Path

# --- Configuration -----------------------------------------------------------
MODULES = ["liara-interfaces", "liara-core", "liara-renderer"]
DEFAULT_PRESETS = {
    "Linux": "linux-debug-clang",
    "Windows": "windows-release"
}

# --- Terminal Colors ---------------------------------------------------------
class Term:
    GREEN = '\033[92m' if os.name == 'posix' or os.getenv('ANSICON') else ''
    YELLOW = '\033[93m' if os.name == 'posix' or os.getenv('ANSICON') else ''
    RED = '\033[91m' if os.name == 'posix' or os.getenv('ANSICON') else ''
    BOLD = '\033[1m' if os.name == 'posix' or os.getenv('ANSICON') else ''
    RESET = '\033[0m' if os.name == 'posix' or os.getenv('ANSICON') else ''

def info(msg):  print(f"\n{Term.BOLD}==> {msg}{Term.RESET}")
def ok(msg):    print(f"  [{Term.GREEN}OK{Term.RESET}] {msg}")
def warn(msg):  print(f"  [{Term.YELLOW}WARN{Term.RESET}] {msg}")
def fatal(msg):
    print(f"  [{Term.RED}FAIL{Term.RESET}] {msg}", file=sys.stderr)
    sys.exit(1)

# --- Command Execution Utilities ---------------------------------------------
def run_cmd(args, cwd=None, capture=False, env=None):
    merged_env = os.environ.copy()
    if env:
        merged_env.update(env)
    try:
        res = subprocess.run(args, cwd=cwd, capture_output=capture, text=True, check=True, env=merged_env)
        return res.returncode, res.stdout
    except subprocess.CalledProcessError as e:
        return e.returncode, e.stdout if capture else ""
    except FileNotFoundError:
        return -1, ""

def has_tool(name):
    return shutil.which(name) is not None

def get_tool_version(args):
    code, out = run_cmd(args, capture=True)
    if code != 0 or not out:
        return None
    import re
    match = re.search(r'(\d+(\.\d+)+)', out)
    return match.group(1) if match else None

def version_ge(actual, minimum):
    from sys import version_info
    actual_parts = [int(x) for x in actual.split('.')]
    min_parts = [int(x) for x in minimum.split('.')]
    return actual_parts >= min_parts

# --- Verification Logic ------------------------------------------------------

def do_verify(args):
    print(f"{Term.BOLD}Liara Engine - Environment Verification{Term.RESET}")
    is_windows = (platform.system() == "Windows")

    def check_req(name, min_ver=None, ver_args=None):
        if not has_tool(name):
            fatal(f"{name}: not found")
        if min_ver and ver_args:
            ver = get_tool_version(ver_args)
            if not ver:
                warn(f"{name}: found, but version could not be parsed")
            elif version_ge(ver, min_ver):
                ok(f"{name}: {ver} (>= {min_ver})")
            else:
                fatal(f"{name}: {ver} is older than required {min_ver}")
        else:
            ok(f"{name}: found")

    def check_optional(name, min_ver=None, ver_args=None):
        if not has_tool(name):
            warn(f"{name}: not found (optional, but recommended)")
            return
        if min_ver and ver_args:
            ver = get_tool_version(ver_args)
            if not ver:
                warn(f"{name}: found, but version could not be parsed")
            elif version_ge(ver, min_ver):
                ok(f"{name} (optional): {ver} (>= {min_ver})")
            else:
                warn(f"{name} (optional): {ver} is older than recommended {min_ver}")
        else:
            ok(f"{name} (optional): found")

    # 1. Core Tools
    info("Core Tools:")
    check_req("git")
    check_req("cmake", "3.29", ["cmake", "--version"])
    if not is_windows:
        check_req("ninja")

    # 2. Compilers
    info("Compilers:")
    has_compiler = False
    if is_windows:
        vswhere = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)")) / "Microsoft Visual Studio/Installer/vswhere.exe"
        if vswhere.exists():
            code, out = run_cmd([str(vswhere), "-latest", "-version", "[17.6,19.0)", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], capture=True)
            if code == 0 and out.strip():
                ok(f"Visual Studio 2022 (17.6+) C++ Tools found at: {out.strip()}")
                has_compiler = True
            else:
                warn("Visual Studio 2022 (17.6+) with C++ workload not found.")
        else:
            warn("vswhere.exe not found.")
    else:
        if has_tool("g++"):
            ver = get_tool_version(["g++", "--version"])
            if ver and version_ge(ver, "14.0"):
                ok(f"g++: {ver} (>= 14)")
                has_compiler = True
            else:
                warn(f"g++: {ver} is older than 14")
        if has_tool("clang++"):
            ver = get_tool_version(["clang++", "--version"])
            if ver and version_ge(ver, "18.0"):
                ok(f"clang++: {ver} (>= 18)")
                has_compiler = True
            else:
                warn(f"clang++: {ver} is older than 18")

    if not has_compiler:
        fatal("No supported C++ compiler found!")

    # 3. vcpkg
    info("vcpkg:")
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    if not vcpkg_root:
        fatal("VCPKG_ROOT env var is not set.")
    vcpkg_path = Path(vcpkg_root)
    if not (vcpkg_path / "scripts/buildsystems/vcpkg.cmake").exists():
        fatal(f"VCPKG_ROOT does not point to a valid vcpkg checkout: {vcpkg_root}")
    ok(f"VCPKG_ROOT: {vcpkg_root}")

    # 4. Vulkan
    info("Vulkan SDK:")
    if is_windows and not os.environ.get("VULKAN_SDK"):
        fatal("VULKAN_SDK environment variable is not set.")
    check_req("glslc")
    if has_tool("vulkaninfo"):
        code, out = run_cmd(["vulkaninfo", "--summary"], capture=True)
        if "apiVersion" in out or "Vulkan Instance Version" in out:
            ok("Vulkan 1.3+ support confirmed by vulkaninfo")
        else:
            warn("vulkaninfo summary output format unexpected or failed to query drivers.")
    else:
        fatal("vulkaninfo: not found (Vulkan SDK missing or not in PATH)")

    # 5. Optional tools
    if args.optional:
        info("Optional Development Tools:")

        check_optional("clang-format", "18.0", ["clang-format", "--version"])
        check_optional("clang-tidy", "18.0", ["clang-tidy", "--version"])

        if not is_windows:
            check_optional("mold", None, ["mold", "--version"])
            check_optional("cargo", None, ["cargo", "--version"])
            check_optional("zig", None, ["zig", "version"])
            check_optional("ccache", None, ["ccache", "--version"])
        else:
            check_optional("cargo", None, ["cargo", "--version"])
            check_optional("zig", None, ["zig", "version"])
            check_optional("ccache", None, ["ccache", "--version"])
            check_optional("sccache", None, ["sccache", "--version"])

    # 6. Smoke Test
    info("CMake Smoke Test:")
    with tempfile.TemporaryDirectory(prefix="liara-smoke-") as tmpdir:
        tmp_path = Path(tmpdir)
        (tmp_path / "CMakeLists.txt").write_text(
            "cmake_minimum_required(VERSION 3.29)\n"
            "project(liara_smoke LANGUAGES CXX)\n"
            "set(CMAKE_CXX_STANDARD 20)\n"
            "add_executable(smoke main.cpp)\n"
        )
        (tmp_path / "main.cpp").write_text("int main() { return 0; }\n")

        gen = "Visual Studio 17 2022" if is_windows else "Ninja"
        code, _ = run_cmd(["cmake", "-S", str(tmp_path), "-B", str(tmp_path / "build"), "-G", gen], capture=True)
        if code == 0:
            code_b, _ = run_cmd(["cmake", "--build", str(tmp_path / "build"), "--config", "Release"], capture=True)
            if code_b == 0:
                ok("Trivial C++20 project configured and compiled successfully.")
            else:
                fatal("Smoke test compile failed.")
        else:
            fatal("Smoke test configuration failed.")

    print(f"\n{Term.BOLD}Verification Complete: {Term.GREEN}Environment is ready!{Term.RESET}")


def do_setup(args):
    script_dir = Path(__file__).resolve().parent
    meta_root = script_dir.parent
    workspace = meta_root / "workspace"
    workspace.mkdir(exist_ok=True)

    git_base = os.environ.get("LIARA_GIT_BASE")
    if not git_base:
        git_base = "git@github.com:liara-engine" if args.ssh else "https://github.com/liara-engine"

    # 1. Clone / Pull Modules
    for module in MODULES:
        module_dir = workspace / module
        if (module_dir / ".git").exists():
            if not args.no_pull:
                info(f"Updating {module}...")
                code, _ = run_cmd(["git", "-C", str(module_dir), "pull", "--ff-only"])
                if code != 0:
                    warn(f"Fast-forward pull failed for {module}. Left as-is.")
            else:
                info(f"Skipping update for {module}")
        else:
            info(f"Cloning {module}...")
            code, _ = run_cmd(["git", "clone", f"{git_base}/{module}.git", str(module_dir)])
            if code != 0:
                fatal(f"Failed to clone {module}")

    # 2. CMake Superbuild Generation
    info("Generating workspace CMakeLists.txt...")
    buildable = [m for m in MODULES if (workspace / m / "CMakeLists.txt").exists()]
    if not buildable:
        fatal("No buildable modules found.")

    cmake_content = (
        "# GENERATED FILE - DO NOT EDIT\n"
        "cmake_minimum_required(VERSION 3.29)\n"
        "project(LiaraWorkspace LANGUAGES C CXX)\n"
        "set(CMAKE_C_STANDARD 11)\n"
        "set(CMAKE_CXX_STANDARD 20)\n"
        "enable_testing()\n\n"
    )
    for m in buildable:
        cmake_content += f"add_subdirectory({m})\n"
    cmake_content += (
        "\nif(EXISTS \"${CMAKE_CURRENT_SOURCE_DIR}/../launcher/CMakeLists.txt\")\n"
        "    add_subdirectory(\"${CMAKE_CURRENT_SOURCE_DIR}/../launcher\" launcher)\n"
        "endif()\n"
    )
    (workspace / "CMakeLists.txt").write_text(cmake_content, encoding="utf-8")

    # 3. Merge Manifest vcpkg
    info("Generating merged vcpkg.json...")
    merged_manifest = {
        "$comment": "GENERATED - Do not edit.",
        "name": "liara-workspace",
        "version-string": "0.0.0",
        "dependencies": [],
        "features": {}
    }

    seen_deps = {}
    for m in buildable:
        vcpkg_path = workspace / m / "vcpkg.json"
        if not vcpkg_path.exists():
            continue
        try:
            data = json.loads(vcpkg_path.read_text(encoding="utf-8"))
            for dep in data.get("dependencies", []):
                key = dep if isinstance(dep, str) else dep.get("name")
                seen_deps[key] = dep

            for f_name, f_body in data.get("features", {}).items():
                feat = merged_manifest["features"].setdefault(f_name, {"description": f_body.get("description", f_name), "dependencies": []})
                existing = {d if isinstance(d, str) else d.get("name") for d in feat["dependencies"]}
                for dep in f_body.get("dependencies", []):
                    key = dep if isinstance(dep, str) else dep.get("name")
                    if key not in existing:
                        feat["dependencies"].append(dep)
        except Exception as e:
            warn(f"Failed to parse vcpkg.json for {m}: {e}")

    merged_manifest["dependencies"] = [seen_deps[k] for k in sorted(seen_deps.keys())]
    if not merged_manifest["features"]:
        del merged_manifest["features"]

    (workspace / "vcpkg.json").write_text(json.dumps(merged_manifest, indent=2) + "\n", encoding="utf-8")

    for m in buildable:
        cfg = workspace / m / "vcpkg-configuration.json"
        if cfg.exists():
            shutil.copy(cfg, workspace / "vcpkg-configuration.json")
            break

    # 4. Generate CMakePresets.json
    info("Generating CMakePresets.json...")
    presets_template = (script_dir / "CMakePresets.json.template")
    presets_data = {
        "version": 6,
        "cmakeMinimumRequired": { "major": 3, "minor": 29, "patch": 0 },
        "configurePresets": [
            {
                "name": "common", "hidden": True,
                "binaryDir": "${sourceDir}/build/${presetName}",
                "cacheVariables": {
                    "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
                    "VCPKG_MANIFEST_FEATURES": "tests",
                    "LIARA_INTERFACES_BUILD_TESTS": "ON",
                    "LIARA_CORE_BUILD_TESTS": "ON",
                    "LIARA_RENDERER_BUILD_TESTS": "ON",
                    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
                }
            },
            { "name": "linux-base", "hidden": True, "inherits": "common", "generator": "Ninja", "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Linux" } },
            { "name": "windows-base", "hidden": True, "inherits": "common", "generator": "Visual Studio 17 2022", "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" } },
            { "name": "linux-debug-gcc", "inherits": "linux-base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "CMAKE_C_COMPILER": "gcc", "CMAKE_CXX_COMPILER": "g++" } },
            { "name": "linux-release-gcc", "inherits": "linux-base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "CMAKE_C_COMPILER": "gcc", "CMAKE_CXX_COMPILER": "g++" } },
            { "name": "linux-debug-clang", "inherits": "linux-base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "CMAKE_C_COMPILER": "clang", "CMAKE_CXX_COMPILER": "clang++" } },
            { "name": "linux-release-clang", "inherits": "linux-base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "CMAKE_C_COMPILER": "clang", "CMAKE_CXX_COMPILER": "clang++" } },
            { "name": "windows-debug", "inherits": "windows-base" },
            { "name": "windows-release", "inherits": "windows-base" }
        ],
        "buildPresets": [
            { "name": "linux-debug-gcc", "configurePreset": "linux-debug-gcc" },
            { "name": "linux-release-gcc", "configurePreset": "linux-release-gcc" },
            { "name": "linux-debug-clang", "configurePreset": "linux-debug-clang" },
            { "name": "linux-release-clang", "configurePreset": "linux-release-clang" },
            { "name": "windows-debug", "configurePreset": "windows-debug", "configuration": "Debug" },
            { "name": "windows-release", "configurePreset": "windows-release", "configuration": "Release" }
        ],
        "testPresets": [
            { "name": "common-test", "hidden": True, "output": { "outputOnFailure": True }, "execution": { "noTestsAction": "error", "stopOnFailure": False } },
            { "name": "linux-debug-gcc", "configurePreset": "linux-debug-gcc", "inherits": "common-test" },
            { "name": "linux-release-gcc", "configurePreset": "linux-release-gcc", "inherits": "common-test" },
            { "name": "linux-debug-clang", "configurePreset": "linux-debug-clang", "inherits": "common-test" },
            { "name": "linux-release-clang", "configurePreset": "linux-release-clang", "inherits": "common-test" },
            { "name": "windows-debug", "configurePreset": "windows-debug", "inherits": "common-test", "configuration": "Debug" },
            { "name": "windows-release", "configurePreset": "windows-release", "inherits": "common-test", "configuration": "Release" }
        ]
    }
    (workspace / "CMakePresets.json").write_text(json.dumps(presets_data, indent=2), encoding="utf-8")

    # 5. Configure CMake
    preset_to_use = args.preset or DEFAULT_PRESETS[platform.system()]
    if not args.no_configure:
        info(f"Configuring CMake with preset '{preset_to_use}'...")
        code, _ = run_cmd(["cmake", "-S", str(workspace), "--preset", preset_to_use])
        if code != 0:
            fatal("CMake configuration failed.")

        if platform.system() != "Windows":
            comp_db = workspace / "build" / preset_to_use / "compile_commands.json"
            if comp_db.exists():
                symlink_path = workspace / "compile_commands.json"
                if symlink_path.exists() or symlink_path.is_symlink():
                    symlink_path.unlink()
                symlink_path.symlink_to(f"build/{preset_to_use}/compile_commands.json")
                ok("Created compile_commands.json symlink.")
    else:
        info("Skipping CMake configure step.")


def do_build(args):
    script_dir = Path(__file__).resolve().parent
    workspace = script_dir.parent / "workspace"
    preset_to_use = args.preset or DEFAULT_PRESETS[platform.system()]

    info(f"Building workspace with preset '{preset_to_use}'...")
    code, _ = run_cmd(["cmake", "--build", "--preset", preset_to_use], cwd=workspace)
    if code != 0:
        fatal("Build failed.")
    ok("Build succeeded!")


def do_test(args):
    script_dir = Path(__file__).resolve().parent
    workspace = script_dir.parent / "workspace"
    preset_to_use = args.preset or DEFAULT_PRESETS[platform.system()]

    info(f"Running tests with preset '{preset_to_use}'...")
    code, _ = run_cmd(["ctest", "--preset", preset_to_use], cwd=workspace)
    if code != 0:
        fatal("Some tests failed.")
    ok("All tests passed!")


def do_launch(args):
    script_dir = Path(__file__).resolve().parent
    workspace = script_dir.parent / "workspace"
    preset_to_use = args.preset or DEFAULT_PRESETS[platform.system()]

    info(f"Launching Liara Engine with preset '{preset_to_use}'...")
    build_dir = workspace / "build" / preset_to_use
    if platform.system() == "Windows":
        exe_path = build_dir / "launcher" / "Debug" / "liara_launcher.exe" if preset_to_use.endswith("debug") else build_dir / "launcher" / "Release" / "liara_launcher.exe"
    else:
        exe_path = build_dir / "launcher" / "liara_launcher"

    if not exe_path.exists():
        fatal(f"Executable not found: {exe_path}. Please build the project first.")

    code, _ = run_cmd([str(exe_path)], cwd=build_dir)
    if code != 0:
        fatal("Liara Engine exited with an error.")
    ok("Liara Engine exited successfully.")


def do_clean(args):
    script_dir = Path(__file__).resolve().parent
    workspace = script_dir.parent / "workspace"
    preset_to_use = args.preset or DEFAULT_PRESETS[platform.system()]

    info(f"Cleaning build artifacts with preset '{preset_to_use}'...")
    code, _ = run_cmd(["cmake", "--build", "--preset", preset_to_use, "--target", "clean"], cwd=workspace)
    if code != 0:
        fatal("Clean failed.")
    ok("Clean succeeded!")


def main():
    parser = argparse.ArgumentParser(description="Liara Engine unified CLI orchestrator")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Sub-command: verify
    verify_parser = subparsers.add_parser("verify", help="Verify toolchain environment")
    verify_parser.add_argument("--optional", action="store_true", help="Check also optional development tools (clang-tidy, mold, ccache...)")

    # Sub-command: setup
    setup_parser = subparsers.add_parser("setup", help="Bootstrap and configure the workspace")
    setup_parser.add_color = True
    setup_parser.add_argument("--ssh", action="store_true", help="Clone over SSH instead of HTTPS")
    setup_parser.add_argument("--preset", help="Override the default CMake preset to configure")
    setup_parser.add_argument("--no-configure", action="store_true", help="Skip running cmake configure")
    setup_parser.add_argument("--no-pull", action="store_true", help="Skip pulling existing clones")

    # Sub-command: build
    build_parser = subparsers.add_parser("build", help="Build the workspace using CMake presets")
    build_parser.add_argument("--preset", help="Override CMake build preset")

    # Sub-command: test
    test_parser = subparsers.add_parser("test", help="Run CTest suite")
    test_parser.add_argument("--preset", help="Override CMake test preset")

    # Sub-command: launch
    launch_parser = subparsers.add_parser("launch", help="Launch the Liara Engine application")
    launch_parser.add_argument("--preset", help="Override CMake build preset for launching")

    # Sub-command: clean
    clean_parser = subparsers.add_parser("clean", help="Clean build artifacts")
    clean_parser.add_argument("--preset", help="Override CMake build preset for cleaning")

    args = parser.parse_args()

    if args.command == "verify":
        do_verify(args)
    elif args.command == "setup":
        do_setup(args)
    elif args.command == "build":
        do_build(args)
    elif args.command == "test":
        do_test(args)
    elif args.command == "launch":
        do_launch(args)
    elif args.command == "clean":
        do_clean(args)

if __name__ == "__main__":
    main()