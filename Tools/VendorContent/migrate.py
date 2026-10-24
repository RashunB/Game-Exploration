"""Copy the union of measured dependency closures from the staging project into Takeover.

Usage: migrate.py <src Content> <dst Content> <dry|copy> <closure.json> [<closure.json> ...]
Checks every package for /Script modules outside the engine allowlist and for non-/Game mount points,
refuses to overwrite differing files, then copies each package (plus any .uexp/.ubulk sidecars)
to the same /Game path.
"""
import filecmp
import json
import os
import re
import shutil
import sys

ENGINE_MODULES = {"/Script/BlueprintGraph", "/Script/CoreUObject", "/Script/Engine", "/Script/Foliage",
                  "/Script/NavigationSystem", "/Script/PhysicsCore", "/Script/UnrealEd", "/Script/Landscape",
                  "/Script/Niagara", "/Script/MovieScene", "/Script/MovieSceneTracks", "/Script/LevelSequence",
                  "/Script/InterchangeEngine", "/Script/InterchangeCore", "/Script/InterchangeImport",
                  "/Script/InterchangePipelines", "/Script/MeshDescription", "/Script/StaticMeshDescription",
                  "/Script/Chaos", "/Script/ChaosCore", "/Script/GeometryCollectionEngine"}
SCRIPT = re.compile(rb"/Script/[A-Za-z0-9_]+")
MOUNT = re.compile(rb"\x00(/(?!Game/|Engine/|Script/)[A-Z][A-Za-z0-9_]+/[A-Za-z0-9_/]+)")


def package_files(content, package):
    base = os.path.join(content, *package[len("/Game/"):].split("/"))
    return [base + ext for ext in (".uasset", ".umap", ".uexp", ".ubulk", ".uptnl") if os.path.isfile(base + ext)]


def main():
    src, dst, mode, closures = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:]
    packages = set()
    for c in closures:
        packages |= set(json.load(open(c, encoding="utf-8"))["files"])
    foreign_scripts, mounts, clashes, plan = {}, {}, [], []
    for package in sorted(packages):
        files = package_files(src, package)
        for f in files:
            if f.endswith((".uasset", ".umap")):
                data = open(f, "rb").read()
                for s in set(SCRIPT.findall(data)):
                    s = s.decode()
                    if s not in ENGINE_MODULES:
                        foreign_scripts.setdefault(s, []).append(package)
                for m in set(MOUNT.findall(data)):
                    mounts.setdefault(m.decode().split("/")[1], []).append(package)
            target = os.path.join(dst, os.path.relpath(f, src))
            if os.path.exists(target) and not filecmp.cmp(f, target, shallow=False):
                clashes.append(target)
            plan.append((f, target))
    total = sum(os.path.getsize(f) for f, _ in plan)
    report = {"packages": len(packages), "files": len(plan), "total_gib": round(total / 2**30, 2),
              "non_engine_scripts": {k: v[:3] for k, v in foreign_scripts.items()},
              "other_mounts": {k: v[:3] for k, v in mounts.items()}, "clashes": clashes[:10]}
    print(json.dumps(report, indent=1))
    if mode != "copy":
        return
    if clashes:
        sys.exit("refusing to copy: existing files differ")
    copied = skipped = 0
    for f, target in plan:
        # Identical files are already in place (and may be held open by the editor)
        if os.path.exists(target) and filecmp.cmp(f, target, shallow=False):
            skipped += 1
            continue
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copy2(f, target)
        copied += 1
    print("copied", copied, "files, skipped", skipped, "identical")


if __name__ == "__main__":
    main()
