import shutil
import pathlib

def handle_resources(resources, module_dir):
    print("    Handling resources...")

    resource_map = {
        "css": "css",
        "js": "js",
        "favicon-ico": "favicon.ico",
        "favicon-svg": "favicon.svg"
    }

    for key, target in resource_map.items():
        if key not in resources:
            continue

        paths = resources[key]
        if isinstance(paths, str):
            paths = [paths]

        for source_path_str in paths:
            source_path = pathlib.Path(source_path_str)

            if not source_path.exists():
                print(f"    WARNING: Resource not found: {source_path_str}")
                continue

            dest_path = module_dir / (target if key.startswith("favicon") else f"{target}/{source_path.name}")
            dest_path.parent.mkdir(parents=True, exist_ok=True)

            shutil.copy2(source_path, dest_path)
            print(f"    Copied {source_path.name} to {dest_path.relative_to(module_dir)}")