"""Dependency closure of Unreal packages by scanning /Game/ references in .uasset/.umap name tables.

Usage: dep_closure.py <ContentDir> <out.json> <seed> [<seed> ...]
A seed is a /Game/ package path, or a /Game/ folder ending in '/' (every package under it).
Writes the closure (package -> bytes), total size, and references that resolve to no file.
"""
import json
import os
import re
import sys

REF = re.compile(rb"/Game/[A-Za-z0-9_\-/]+")


def package_file(content, package):
    base = os.path.join(content, *package[len("/Game/"):].split("/"))
    for ext in (".uasset", ".umap"):
        if os.path.isfile(base + ext):
            return base + ext
    return None


def expand_seed(content, seed):
    if not seed.endswith("/"):
        return [seed]
    root = os.path.join(content, *seed[len("/Game/"):].strip("/").split("/"))
    out = []
    for dirpath, _, files in os.walk(root):
        for name in files:
            if name.endswith((".uasset", ".umap")):
                rel = os.path.relpath(os.path.join(dirpath, os.path.splitext(name)[0]), content)
                out.append("/Game/" + rel.replace(os.sep, "/"))
    return out


def main():
    content, out_path, seeds = sys.argv[1], sys.argv[2], sys.argv[3:]
    pending = [p for s in seeds for p in expand_seed(content, s)]
    sizes, missing = {}, set()
    while pending:
        package = pending.pop()
        if package in sizes or package in missing:
            continue
        path = package_file(content, package)
        if not path:
            missing.add(package)
            continue
        sizes[package] = os.path.getsize(path)
        with open(path, "rb") as f:
            data = f.read()
        for ref in set(REF.findall(data)):
            ref = ref.decode("ascii").rstrip("/")
            # Name tables also hold folder paths; keep only references that resolve to a package
            if ref in sizes:
                continue
            if package_file(content, ref):
                pending.append(ref)
                continue
            # FNames store a trailing _N (no leading zero) as a separate number, so the table
            # holds "MI_Wall_01" for the package "MI_Wall_01_0": try the numbered siblings
            folder = os.path.join(content, *ref[len("/Game/"):].split("/")[:-1])
            stem = ref.rsplit("/", 1)[-1]
            if os.path.isdir(folder):
                for name in os.listdir(folder):
                    base, ext = os.path.splitext(name)
                    if ext in (".uasset", ".umap") and re.fullmatch(re.escape(stem) + r"_(0|[1-9][0-9]*)", base):
                        pending.append(ref.rsplit("/", 1)[0] + "/" + base)
    total = sum(sizes.values())
    top = {}
    for package, size in sizes.items():
        key = "/".join(package.split("/")[:4])
        top[key] = top.get(key, 0) + size
    report = {"packages": len(sizes), "total_mb": round(total / 2**20, 1),
              "by_folder_mb": {k: round(v / 2**20, 1) for k, v in sorted(top.items(), key=lambda kv: -kv[1])},
              "largest": sorted(((round(v / 2**20, 1), k) for k, v in sizes.items()), reverse=True)[:12],
              "files": sorted(sizes)}
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=1)
    print(json.dumps({k: report[k] for k in ("packages", "total_mb", "by_folder_mb", "largest")}, indent=1))


if __name__ == "__main__":
    main()
