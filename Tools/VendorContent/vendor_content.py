"""Third-party Content is kept out of git and restored from its sources by this manifest.

City Sample and Fab content is licensed for use in Unreal Engine products, not for publishing in a
public repository, and the vehicle template ships with the engine. The repo commits only
manifest.json: every third-party file under Content/ with its source and SHA-256.

Usage (from the repo root; Python 3.9+, the engine's own Python works):
  vendor_content.py restore [--staging DIR] [--engine DIR] [--force]
      Copy every manifest file that is missing. Files that exist and differ are reported and left
      alone unless --force.
  vendor_content.py verify [--staging DIR] [--engine DIR]
      Report missing, differing, and unrecorded third-party files. Exit 1 on any problem.
  vendor_content.py record [--staging DIR] [--engine DIR]
      Rewrite the manifest from the third-party files now in Content/. Run after migrating new
      content. Refuses files that differ from their source, since third-party content is never edited.

--staging is the staging project's Content folder (default: $TAKEOVER_STAGING_CONTENT, then
C:/project_workspace/Unreal Engine/CitySample/Content). --engine is the engine install root
(default: $UE_ROOT, then C:/Program Files/Epic Games/UE_5.8).
"""
import argparse
import hashlib
import json
import os
import shutil
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
MANIFEST = os.path.join(os.path.dirname(os.path.abspath(__file__)), "manifest.json")

# Content folders that belong to this project and are committed. Everything else under Content/
# is third-party, except per-user editor folders that are never committed.
FIRST_PARTY = ("Takeover", "__ExternalActors__", "__ExternalObjects__")
LOCAL_ONLY = ("Developers", "Collections")

# Source of each third-party Content folder, first match wins: (Content prefix, source, path inside the source)
SOURCE_RULES = (
	("Vehicles/", "engine-vehicles", ""),
	("", "staging", ""),
)


def sha256(path):
	digest = hashlib.sha256()
	with open(path, "rb") as f:
		for chunk in iter(lambda: f.read(1 << 20), b""):
			digest.update(chunk)
	return digest.hexdigest()


def source_roots(args):
	staging = args.staging or os.environ.get("TAKEOVER_STAGING_CONTENT") or "C:/project_workspace/Unreal Engine/CitySample/Content"
	engine = args.engine or os.environ.get("UE_ROOT") or "C:/Program Files/Epic Games/UE_5.8"
	return {
		"staging": staging,
		"engine-vehicles": os.path.join(engine, "Templates", "TemplateResources", "Standard", "Vehicles", "Content"),
	}


def source_of(content_rel):
	for prefix, source, inner in SOURCE_RULES:
		if content_rel.startswith(prefix):
			return source, inner + content_rel[len(prefix):]
	raise ValueError(content_rel)


def third_party_files():
	content = os.path.join(REPO, "Content")
	for top in sorted(os.listdir(content)):
		if top in FIRST_PARTY or top in LOCAL_ONLY or not os.path.isdir(os.path.join(content, top)):
			continue
		for dirpath, _, names in os.walk(os.path.join(content, top)):
			for name in names:
				yield os.path.relpath(os.path.join(dirpath, name), content).replace(os.sep, "/")


def load_manifest():
	with open(MANIFEST, encoding="utf-8") as f:
		return json.load(f)["files"]


def record(roots, _force):
	files, problems = {}, []
	for rel in third_party_files():
		source, path = source_of(rel)
		src = os.path.join(roots[source], path)
		digest = sha256(os.path.join(REPO, "Content", rel))
		if not os.path.isfile(src):
			problems.append("no source file: " + rel)
		elif sha256(src) != digest:
			problems.append("differs from source: " + rel)
		files["Content/" + rel] = {"source": source, "path": path, "size": os.path.getsize(src) if os.path.isfile(src) else 0, "sha256": digest}
	if problems:
		print("\n".join(problems))
		return 1
	with open(MANIFEST, "w", encoding="utf-8", newline="\n") as f:
		json.dump({"files": files}, f, indent=1, sort_keys=True)
		f.write("\n")
	print("recorded %d files, %.2f GiB" % (len(files), sum(e["size"] for e in files.values()) / 2**30))
	return 0


def restore(roots, force):
	copied = present = 0
	problems = []
	for rel, entry in sorted(load_manifest().items()):
		dst = os.path.join(REPO, rel)
		src = os.path.join(roots[entry["source"]], entry["path"])
		if os.path.isfile(dst):
			if sha256(dst) == entry["sha256"]:
				present += 1
				continue
			if not force:
				problems.append("differs from manifest, left alone (use --force): " + rel)
				continue
		if not os.path.isfile(src):
			problems.append("missing in source %s: %s" % (entry["source"], src))
			continue
		if sha256(src) != entry["sha256"]:
			problems.append("source file is a different version: " + src)
			continue
		os.makedirs(os.path.dirname(dst), exist_ok=True)
		shutil.copy2(src, dst)
		copied += 1
	print("copied %d, already present %d" % (copied, present))
	print("\n".join(problems))
	return 1 if problems else 0


def verify(_roots, _force):
	manifest = load_manifest()
	problems = []
	for rel, entry in sorted(manifest.items()):
		path = os.path.join(REPO, rel)
		if not os.path.isfile(path):
			problems.append("missing: " + rel)
		elif sha256(path) != entry["sha256"]:
			problems.append("differs: " + rel)
	for rel in third_party_files():
		if "Content/" + rel not in manifest:
			problems.append("not in manifest (run record): Content/" + rel)
	print("%d manifest files, %d problems" % (len(manifest), len(problems)))
	print("\n".join(problems))
	return 1 if problems else 0


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument("command", choices=("restore", "verify", "record"))
	parser.add_argument("--staging", help="staging project's Content folder")
	parser.add_argument("--engine", help="engine install root")
	parser.add_argument("--force", action="store_true", help="restore: overwrite files that differ from the manifest")
	args = parser.parse_args()
	command = {"restore": restore, "verify": verify, "record": record}[args.command]
	sys.exit(command(source_roots(args), args.force))


if __name__ == "__main__":
	main()
