"""Tests for repo_checks.py. Each test builds a throwaway git repo. Run: python -m pytest .github/scripts"""
import json
import os
import shutil
import subprocess
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import repo_checks  # noqa: E402

POINTER = b"version https://git-lfs.github.com/spec/v1\noid sha256:" + b"a" * 64 + b"\nsize 12345\n"
VALID_ENTRY = {"path": "CitySample/A.uasset", "source": "staging", "size": 10, "sha256": "b" * 64}


def git(root, *args):
	subprocess.run(("git",) + args, cwd=root, check=True, capture_output=True)


def write(root, path, data):
	full = os.path.join(root, path)
	os.makedirs(os.path.dirname(full), exist_ok=True)
	with open(full, "wb") as f:
		f.write(data if isinstance(data, bytes) else data.encode())


@pytest.fixture
def repo(tmp_path):
	root = str(tmp_path)
	git(root, "init", "-q")
	# Store LFS-pattern files exactly as written, so tests control whether they are pointers
	git(root, "config", "filter.lfs.clean", "cat")
	git(root, "config", "filter.lfs.smudge", "cat")
	git(root, "config", "filter.lfs.process", "")
	git(root, "config", "filter.lfs.required", "false")
	write(root, ".gitattributes", "*.uasset filter=lfs diff=lfs merge=lfs -text\n")
	os.makedirs(os.path.join(root, "Tools", "VendorContent"))
	shutil.copy(os.path.join(REPO, "Tools", "VendorContent", "vendor_content.py"), os.path.join(root, "Tools", "VendorContent"))
	write(root, "Tools/VendorContent/manifest.json", json.dumps({"files": {"Content/CitySample/A.uasset": VALID_ENTRY}}))
	return root


def problems(root, name):
	git(root, "add", "-A", "-f")
	return repo_checks.CHECKS[name](root, repo_checks.tracked(root))


def test_content_allows_first_party_folders(repo):
	write(repo, "Content/Takeover/Data/DA.uasset", POINTER)
	write(repo, "Content/__ExternalActors__/L/A.uasset", POINTER)
	write(repo, "Content/__ExternalObjects__/L/B.uasset", POINTER)
	assert problems(repo, "content") == []


def test_content_blocks_third_party_and_root_files(repo):
	write(repo, "Content/CitySample/Dummy.txt", "x")
	write(repo, "Content/Stray.txt", "x")
	write(repo, "Content/TakeoverExtra/A.txt", "x")
	assert len(problems(repo, "content")) == 3


def test_manifest_accepts_complete_entries(repo):
	assert problems(repo, "manifest") == []


@pytest.mark.parametrize("change", [
	{"sha256": "not-a-hash"},
	{"source": "fab"},
	{"path": ""},
	{"size": -1},
])
def test_manifest_rejects_incomplete_entries(repo, change):
	write(repo, "Tools/VendorContent/manifest.json", json.dumps({"files": {"Content/CitySample/A.uasset": dict(VALID_ENTRY, **change)}}))
	assert len(problems(repo, "manifest")) == 1


def test_manifest_rejects_first_party_keys(repo):
	write(repo, "Tools/VendorContent/manifest.json", json.dumps({"files": {"Content/Takeover/A.uasset": VALID_ENTRY}}))
	assert len(problems(repo, "manifest")) == 1


def test_manifest_rejects_unparsable_file(repo):
	write(repo, "Tools/VendorContent/manifest.json", "{")
	assert len(problems(repo, "manifest")) == 1


def test_lfs_accepts_pointers(repo):
	write(repo, "Content/Takeover/A.uasset", POINTER)
	assert problems(repo, "lfs") == []


def test_lfs_rejects_small_and_large_binaries(repo):
	write(repo, "Content/Takeover/Small.uasset", b"\x00binary")
	write(repo, "Content/Takeover/Large.uasset", b"\x00" * 4096)
	assert len(problems(repo, "lfs")) == 2


def test_size_rejects_big_files_outside_lfs(repo):
	write(repo, "Source/Big.bin", b"\x00" * (repo_checks.MAX_BLOB_BYTES + 1))
	assert len(problems(repo, "size")) == 1


def test_size_exempts_manifest_and_lfs_files(repo):
	write(repo, "Tools/VendorContent/manifest.json", json.dumps({"files": {}, "pad": "x" * (repo_checks.MAX_BLOB_BYTES + 1)}))
	write(repo, "Content/Takeover/Big.uasset", b"\x00" * (repo_checks.MAX_BLOB_BYTES + 1))
	assert problems(repo, "size") == []


def test_links_resolve_relative_paths_and_skip_externals(repo):
	write(repo, "docs/a.md", "x")
	write(repo, "docs/sub/b.md", "[up](../a.md) [anchor](../a.md#part) [web](https://example.com) [mail](mailto:x@y.z) [self](#top)")
	assert problems(repo, "links") == []


def test_links_report_broken_targets(repo):
	write(repo, "docs/b.md", "line one\n[gone](missing.md)\n")
	assert problems(repo, "links") == ["docs/b.md:2: broken link missing.md"]


def test_links_ignore_code(repo):
	write(repo, "docs/c.md", "```\n[x](missing.md)\n```\n`[y](missing.md)`\n")
	assert problems(repo, "links") == []


def test_links_skip_vendored_skills(repo):
	write(repo, ".claude/skills/ue-thing/SKILL.md", "[x](missing.md)")
	assert problems(repo, "links") == []
