"""Cheap repository checks run by CI on every push and pull request, and runnable locally.

GitHub-hosted runners cannot build Unreal, so CI guards what a bad commit to a public repo could break:
  content    no tracked file under Content/ outside this project's own folders (the license boundary)
  manifest   Tools/VendorContent/manifest.json parses and every entry is complete
  lfs        every file matching an LFS pattern in .gitattributes is committed as an LFS pointer
  size       no blob over 1 MiB outside LFS, except the manifest
  links      relative links in Markdown resolve (offline; external links are not checked)

Usage (from anywhere inside the repo; Python 3.9+, stdlib only):
  repo_checks.py [check ...]     run the named checks, or all of them
Reads what is committed at HEAD plus the index, never the LFS objects, so it works on a checkout
made with lfs: false.
"""
import json
import os
import re
import subprocess
import sys

MAX_BLOB_BYTES = 1 << 20
SIZE_EXEMPT = ("Tools/VendorContent/manifest.json",)
LFS_POINTER_PREFIX = b"version https://git-lfs.github.com/spec/v1\n"
LFS_POINTER_MAX_BYTES = 1024
SHA256 = re.compile(r"^[0-9a-f]{64}$")

# Markdown checked for links. Vendored third-party skills are left out: their links are upstream's.
LINK_ROOTS = ("AGENTS.md", "CLAUDE.md", "PLAN.md", "CHANGELOG.md", "README.md", "docs", ".github", ".claude", ".agents")
LINK_EXCLUDE = re.compile(r"^\.claude/skills/ue-")
MD_LINK = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
FENCE = re.compile(r"^\s*(```|~~~)")
INLINE_CODE = re.compile(r"`[^`]*`")


def git(root, *args, data=None):
	return subprocess.run(("git",) + args, cwd=root, input=data, capture_output=True, check=True).stdout


def repo_root():
	return git(os.getcwd(), "rev-parse", "--show-toplevel").decode().strip()


def first_party(root):
	"""Committed Content folders, from the vendor tool so there is one list."""
	sys.path.insert(0, os.path.join(root, "Tools", "VendorContent"))
	try:
		import vendor_content
	finally:
		sys.path.pop(0)
	return vendor_content.FIRST_PARTY


def tracked(root):
	"""(mode, blob sha, path) for every file in the index."""
	entries = []
	for record in git(root, "ls-files", "-s", "-z").split(b"\0"):
		if not record:
			continue
		meta, path = record.split(b"\t", 1)
		mode, sha, _stage = meta.split(b" ")
		entries.append((mode.decode(), sha.decode(), path.decode("utf-8", "surrogateescape")))
	return entries


def lfs_paths(root, paths):
	"""Paths whose filter attribute is lfs."""
	if not paths:
		return set()
	out = git(root, "check-attr", "-z", "--stdin", "filter", data="\0".join(paths).encode("utf-8", "surrogateescape") + b"\0")
	fields = out.split(b"\0")
	result = set()
	for index in range(0, len(fields) - 2, 3):
		if fields[index + 2] == b"lfs":
			result.add(fields[index].decode("utf-8", "surrogateescape"))
	return result


def blob_sizes(root, shas):
	out = git(root, "cat-file", "--batch-check=%(objectname) %(objectsize)", data=("\n".join(shas) + "\n").encode())
	sizes = {}
	for line in out.decode().splitlines():
		sha, size = line.split()
		sizes[sha] = int(size)
	return sizes


def check_content(root, entries):
	allowed = tuple("Content/%s/" % name for name in first_party(root))
	return ["third-party or stray file under Content/: " + path
		for _mode, _sha, path in entries
		if path.startswith("Content/") and not path.startswith(allowed)]


def check_manifest(root, _entries):
	path = os.path.join(root, "Tools", "VendorContent", "manifest.json")
	try:
		with open(path, encoding="utf-8") as f:
			files = json.load(f)["files"]
	except (OSError, ValueError, KeyError) as error:
		return ["manifest.json unreadable: %s" % error]
	if not isinstance(files, dict):
		return ["manifest.json: 'files' is not an object"]
	allowed = tuple("Content/%s/" % name for name in first_party(root))
	sources = {"staging", "engine-vehicles"}
	problems = []
	for key, entry in sorted(files.items()):
		if not isinstance(entry, dict):
			problems.append("manifest entry is not an object: " + key)
			continue
		if not key.startswith("Content/") or key.startswith(allowed):
			problems.append("manifest key is not a third-party Content path: " + key)
		if not isinstance(entry.get("path"), str) or not entry.get("path"):
			problems.append("manifest entry has no path: " + key)
		if entry.get("source") not in sources:
			problems.append("manifest entry has an unknown source %r: %s" % (entry.get("source"), key))
		if not isinstance(entry.get("sha256"), str) or not SHA256.match(entry["sha256"]):
			problems.append("manifest entry has no valid sha256: " + key)
		if not isinstance(entry.get("size"), int) or entry["size"] < 0:
			problems.append("manifest entry has no valid size: " + key)
	return problems


def check_lfs(root, entries):
	files = [(sha, path) for mode, sha, path in entries if mode != "120000" and mode != "160000"]
	lfs = lfs_paths(root, [path for _sha, path in files])
	wanted = [(sha, path) for sha, path in files if path in lfs]
	if not wanted:
		return []
	sizes = blob_sizes(root, sorted({sha for sha, _path in wanted}))
	problems = []
	small = [(sha, path) for sha, path in wanted if sizes[sha] <= LFS_POINTER_MAX_BYTES]
	problems += ["matches an LFS pattern but is not an LFS pointer (%d bytes): %s" % (sizes[sha], path)
		for sha, path in wanted if sizes[sha] > LFS_POINTER_MAX_BYTES]
	for sha, path in small:
		if not git(root, "cat-file", "blob", sha).startswith(LFS_POINTER_PREFIX):
			problems.append("matches an LFS pattern but is not an LFS pointer: " + path)
	return problems


def check_size(root, entries):
	files = [(sha, path) for mode, sha, path in entries if mode != "120000" and mode != "160000"]
	lfs = lfs_paths(root, [path for _sha, path in files])
	sizes = blob_sizes(root, sorted({sha for sha, _path in files}))
	return ["%.1f MiB outside LFS (limit 1 MiB): %s" % (sizes[sha] / 2**20, path)
		for sha, path in files
		if sizes[sha] > MAX_BLOB_BYTES and path not in lfs and path not in SIZE_EXEMPT]


def markdown_files(root, entries):
	for _mode, _sha, path in entries:
		if not path.endswith(".md") or LINK_EXCLUDE.match(path):
			continue
		if any(path == top or path.startswith(top + "/") for top in LINK_ROOTS):
			yield path


def links_in(text):
	in_fence = False
	for number, line in enumerate(text.splitlines(), 1):
		if FENCE.match(line):
			in_fence = not in_fence
			continue
		if in_fence:
			continue
		for match in MD_LINK.finditer(INLINE_CODE.sub("", line)):
			yield number, match.group(1)


def check_links(root, entries):
	problems = []
	for path in markdown_files(root, entries):
		with open(os.path.join(root, path), encoding="utf-8") as f:
			text = f.read()
		for number, target in links_in(text):
			if re.match(r"^[a-z][a-z0-9+.-]*:", target, re.I) or target.startswith("#"):
				continue
			target = target.split("#", 1)[0].split("?", 1)[0]
			base = root if target.startswith("/") else os.path.dirname(os.path.join(root, path))
			if not os.path.exists(os.path.normpath(os.path.join(base, target.lstrip("/")))):
				problems.append("%s:%d: broken link %s" % (path, number, target))
	return problems


CHECKS = {
	"content": check_content,
	"manifest": check_manifest,
	"lfs": check_lfs,
	"size": check_size,
	"links": check_links,
}


def run(root, names):
	entries = tracked(root)
	failed = 0
	for name in names:
		problems = CHECKS[name](root, entries)
		print("%-8s %s" % (name, "ok" if not problems else "%d problem(s)" % len(problems)))
		for problem in problems:
			print("  " + problem)
		failed += bool(problems)
	return 1 if failed else 0


def main(argv):
	names = argv or list(CHECKS)
	unknown = [name for name in names if name not in CHECKS]
	if unknown:
		print("unknown check(s): %s; choose from %s" % (", ".join(unknown), ", ".join(CHECKS)))
		return 2
	return run(repo_root(), names)


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
