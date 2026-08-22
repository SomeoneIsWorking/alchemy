#!/usr/bin/env python3
"""Verify the real Marvel Ultimate Alliance PS2 and Xbox 360 asset corpora.

The gate starts from the two user-supplied disc images.  It does not accept an
already-extracted directory, because that cannot prove which archive layers
were inspected.  All derived files are staged below the repository's ignored
``scratch/`` directory.
"""

from __future__ import annotations

import argparse
import collections
import importlib.util
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

if __package__:
    from .alchemy_archives import AlchemyZip, ArchiveError, extract_fb
else:
    from alchemy_archives import AlchemyZip, ArchiveError, extract_fb

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SCRATCH = ROOT / "scratch" / "mua-corpus"
REQUIRED_ARCHIVES = (
    "ps2:Z/ASSETSFB.WAD",
    "x360:z/assetsfb.zip",
    "x360:muadlc_content.zip",
    "x360:muadlc_titleupdate.zip",
)
_OBJECTS = re.compile(r"\bobjects=(\d+)\b")
_IMAGES = re.compile(r"^images=(\d+)$", re.MULTILINE)


class CorpusRefused(RuntimeError):
    """Required input or tooling is absent, so no corpus claim can be made."""


@dataclass
class CorpusStats:
    archives_opened: int = 0
    archive_entries: int = 0
    archive_counts: dict[str, int] = field(default_factory=dict)
    fb_candidates: int = 0
    fb_parsed: int = 0
    fb_empty: int = 0
    fb_entries: int = 0
    fb_duplicate_paths: int = 0
    xmlb_candidates: int = 0
    xmlb_parsed: int = 0
    xmlb_roundtripped: int = 0
    xmlb_nodes: int = 0
    igb_candidates: int = 0
    igb_opened: int = 0
    igb_objects: int = 0
    igb_images: int = 0
    empty_payloads: collections.Counter[str] = field(default_factory=collections.Counter)
    unsupported: collections.Counter[str] = field(default_factory=collections.Counter)
    failures: list[str] = field(default_factory=list)

    def passed(self, require_igb: bool = True) -> bool:
        enough = (
            self.archives_opened == len(REQUIRED_ARCHIVES)
            and self.fb_candidates > 0
            and self.fb_parsed == self.fb_candidates
            and self.xmlb_candidates > 0
            and self.xmlb_roundtripped == self.xmlb_candidates
        )
        if require_igb:
            enough = enough and self.igb_candidates > 0 and self.igb_opened == self.igb_candidates
        return enough and not self.failures


def _load_xmlb():
    path = ROOT / "tools" / "xmlb.py"
    spec = importlib.util.spec_from_file_location("alchemy_shipping_xmlb", path)
    if spec is None or spec.loader is None:
        raise CorpusRefused(f"cannot load shipping XMLB parser: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _count_nodes(root: object) -> int:
    return 1 + sum(_count_nodes(child) for child in root.children)


def _suffix(name: str) -> str:
    suffix = Path(name.rsplit("!", 1)[-1]).suffix.casefold()
    return suffix or "<no-extension>"


def _inspect_xmlb(path: Path, display: str, stats: CorpusStats, xmlb: object) -> None:
    stats.xmlb_candidates += 1
    try:
        data = path.read_bytes()
        root = xmlb.parse(data)
        stats.xmlb_parsed += 1
        stats.xmlb_nodes += _count_nodes(root)
        if xmlb.serialise(root) != data:
            stats.failures.append(f"XMLB round-trip differs: {display}")
            return
        stats.xmlb_roundtripped += 1
    except Exception as error:
        stats.failures.append(f"XMLB parser refused {display}: {error}")


def _inspect_igb(path: Path, display: str, stats: CorpusStats, igb_dump: Path) -> None:
    stats.igb_candidates += 1
    result = subprocess.run(
        [str(igb_dump), str(path)],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        reason = result.stderr.strip() or result.stdout.strip() or f"exit {result.returncode}"
        stats.failures.append(f"IGB parser refused {display}: {reason}")
        return
    objects = _OBJECTS.search(result.stdout)
    images = _IMAGES.search(result.stdout)
    if objects is None or images is None:
        stats.failures.append(f"IGB parser gave no denominators for {display}")
        return
    stats.igb_opened += 1
    stats.igb_objects += int(objects.group(1))
    stats.igb_images += int(images.group(1))


def _inspect_asset(
    path: Path,
    display: str,
    stats: CorpusStats,
    xmlb: object,
    igb_dump: Path | None,
) -> None:
    suffix = _suffix(display)
    if path.stat().st_size == 0:
        stats.empty_payloads[suffix] += 1
        return
    if suffix == ".xmlb":
        _inspect_xmlb(path, display, stats, xmlb)
    elif suffix == ".igb":
        if igb_dump is None:
            stats.unsupported[".igb (selftest does not fabricate IGB semantics)"] += 1
        else:
            _inspect_igb(path, display, stats, igb_dump)
    else:
        stats.unsupported[suffix] += 1


def _inspect_fb(
    package: Path,
    display: str,
    work_root: Path,
    stats: CorpusStats,
    xmlb: object,
    igb_dump: Path | None,
) -> None:
    stats.fb_candidates += 1
    try:
        with tempfile.TemporaryDirectory(prefix="fb-", dir=work_root) as temp:
            members = extract_fb(package, Path(temp))
            stats.fb_parsed += 1
            stats.fb_entries += len(members)
            counts = collections.Counter(member.name.casefold() for member in members)
            stats.fb_duplicate_paths += sum(count - 1 for count in counts.values())
            if not members:
                stats.fb_empty += 1
            for member in members:
                _inspect_asset(
                    member.path,
                    f"{display}!{member.name}",
                    stats,
                    xmlb,
                    igb_dump,
                )
    except Exception as error:
        stats.failures.append(f"FB parser refused {display}: {error}")


def inspect_archives(
    archives: dict[str, Path],
    work_root: Path,
    igb_dump: Path | None,
) -> CorpusStats:
    xmlb = _load_xmlb()
    stats = CorpusStats()
    for label in REQUIRED_ARCHIVES:
        path = archives.get(label)
        if path is None:
            raise CorpusRefused(f"required archive was not staged: {label}")
        try:
            archive = AlchemyZip(path)
        except ArchiveError as error:
            raise CorpusRefused(f"required archive cannot be inspected: {label}: {error}") from error
        with archive:
            members = archive.members()
            stats.archives_opened += 1
            stats.archive_entries += len(members)
            stats.archive_counts[label] = len(members)
            for index, member in enumerate(members):
                suffix = _suffix(member.name)
                display = f"{label}!{member.name}"
                if suffix not in (".fb", ".xmlb", ".igb"):
                    stats.unsupported[suffix] += 1
                    continue
                materialized = work_root / f"archive-entry-{index}{suffix}"
                archive.materialize(member.name, materialized)
                if suffix == ".fb":
                    _inspect_fb(materialized, display, work_root, stats, xmlb, igb_dump)
                else:
                    _inspect_asset(materialized, display, stats, xmlb, igb_dump)
    return stats


def _required_tool(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        raise CorpusRefused(f"required tool is missing: {name}")
    return path


def _check_iso(path: Path | None, label: str) -> Path:
    if path is None:
        raise CorpusRefused(f"{label} ISO path was not supplied")
    if not path.is_file():
        raise CorpusRefused(f"{label} ISO is missing or not a file: {path}")
    if path.stat().st_size == 0:
        raise CorpusRefused(f"{label} ISO is empty: {path}")
    return path


def _run_extract(command: list[str], destination: Path, stdout_file: bool = False) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if stdout_file:
        with destination.open("wb") as output:
            result = subprocess.run(command, check=False, stdout=output, stderr=subprocess.PIPE)
    else:
        result = subprocess.run(command, check=False, capture_output=True)
    if result.returncode != 0 or not destination.is_file() or destination.stat().st_size == 0:
        stderr = result.stderr.decode("utf-8", "replace").strip()
        raise CorpusRefused(f"archive extraction failed ({' '.join(command[:2])}): {stderr}")


def stage_archives(ps2_iso: Path, x360_iso: Path, work_root: Path) -> dict[str, Path]:
    seven_zip = _required_tool("7z")
    xdvdfs = _required_tool("xdvdfs")
    staged = {
        REQUIRED_ARCHIVES[0]: work_root / "ps2-assetsfb.wad",
        REQUIRED_ARCHIVES[1]: work_root / "x360-assetsfb.zip",
        REQUIRED_ARCHIVES[2]: work_root / "x360-muadlc-content.zip",
        REQUIRED_ARCHIVES[3]: work_root / "x360-muadlc-titleupdate.zip",
    }
    _run_extract(
        [seven_zip, "x", "-so", str(ps2_iso), "Z/ASSETSFB.WAD"],
        staged[REQUIRED_ARCHIVES[0]],
        stdout_file=True,
    )
    for label, source in (
        (REQUIRED_ARCHIVES[1], "z/assetsfb.zip"),
        (REQUIRED_ARCHIVES[2], "muadlc_content.zip"),
        (REQUIRED_ARCHIVES[3], "muadlc_titleupdate.zip"),
    ):
        _run_extract(
            [xdvdfs, "copy-out", str(x360_iso), source, str(staged[label])],
            staged[label],
        )
    return staged


def _find_igb_dump(explicit: Path | None) -> Path:
    candidates = [explicit] if explicit else [
        ROOT / "scratch" / "build-clang" / "igb_dump",
        ROOT / "build" / "igb_dump",
    ]
    for candidate in candidates:
        if candidate is not None and candidate.is_file() and candidate.stat().st_mode & 0o111:
            return candidate
    tried = ", ".join(str(path) for path in candidates if path is not None)
    raise CorpusRefused(f"shipping igb_dump is missing or not executable; tried: {tried}")


def print_report(stats: CorpusStats) -> None:
    print(f"archives: {stats.archives_opened} of {len(REQUIRED_ARCHIVES)} required opened")
    for label in REQUIRED_ARCHIVES:
        print(f"  {label}: {stats.archive_counts.get(label, 0)} file entries")
    print(f"archive file entries: {stats.archive_entries}")
    print(
        f"FB packages: {stats.fb_parsed} of {stats.fb_candidates} parsed; "
        f"{stats.fb_entries} embedded entries; {stats.fb_duplicate_paths} duplicate-path "
        f"occurrences; {stats.fb_empty} empty packages"
    )
    print(
        f"XMLB payloads: {stats.xmlb_roundtripped} of {stats.xmlb_candidates} "
        f"parsed and round-tripped; {stats.xmlb_nodes} nodes"
    )
    print(
        f"IGB containers: {stats.igb_opened} of {stats.igb_candidates} opened; "
        f"{stats.igb_objects} objects; {stats.igb_images} image payloads"
    )
    empty = sum(stats.empty_payloads.values())
    print(f"declared zero-byte payload placeholders: {empty}")
    for suffix, count in sorted(stats.empty_payloads.items()):
        print(f"  {suffix}: {count}")
    unsupported = sum(stats.unsupported.values())
    print(f"unsupported embedded/archive entries: {unsupported}")
    for suffix, count in sorted(stats.unsupported.items()):
        print(f"  {suffix}: {count}")
    print(f"parser failures: {len(stats.failures)}")
    for failure in stats.failures:
        print(f"  FAIL {failure}")


def _write_zip(path: Path, entries: dict[str, bytes], trailing_eocd: bool = False) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as archive:
        for name, data in entries.items():
            archive.writestr(name, data)
    if trailing_eocd:
        with path.open("ab") as output:
            output.write(b"PK\x05\x06")


def selftest(scratch: Path) -> int:
    xmlb = _load_xmlb()
    scratch.mkdir(parents=True, exist_ok=True)
    checks = []
    with tempfile.TemporaryDirectory(prefix="selftest-", dir=scratch) as temp:
        root = Path(temp)
        document = xmlb.serialise(xmlb.Node("root", [("answer", "positive")]))
        raven_path = str(ROOT / "tools" / "raven-formats" / "src")
        if raven_path not in sys.path:
            sys.path.insert(0, raven_path)
        from raven_formats.fb import FBFileHeader, iter_entries

        fb_entry = FBFileHeader.pack(b"inside.xmlb", b"xml", len(document)) + document
        fb_data = fb_entry + fb_entry
        paths = {label: root / f"archive-{index}.zip" for index, label in enumerate(REQUIRED_ARCHIVES)}
        _write_zip(paths[REQUIRED_ARCHIVES[0]], {"packages/test.fb": fb_data}, trailing_eocd=True)
        _write_zip(paths[REQUIRED_ARCHIVES[1]], {"data/base.xmlb": document, "readme.bin": b"?"})
        _write_zip(paths[REQUIRED_ARCHIVES[2]], {"data/dlc.xmlb": document})
        _write_zip(paths[REQUIRED_ARCHIVES[3]], {"data/update.xmlb": document})
        positive = inspect_archives(paths, root, igb_dump=None)
        checks.append(
            (
                "positive archive/FB/XMLB path",
                positive.passed(require_igb=False)
                and positive.fb_entries == 2
                and positive.fb_duplicate_paths == 1
                and positive.xmlb_roundtripped == 5
                and positive.unsupported[".bin"] == 1,
            )
        )

        malformed = root / "malformed.wad"
        malformed.write_bytes(b"not a ZIP")
        try:
            AlchemyZip(malformed)
            refused_malformed = False
        except ArchiveError:
            refused_malformed = True
        checks.append(("malformed archive refusal", refused_malformed))

        missing = root / "missing.iso"
        try:
            _check_iso(missing, "synthetic")
            refused_missing = False
        except CorpusRefused:
            refused_missing = True
        checks.append(("missing corpus refusal", refused_missing))

        truncated_fb = FBFileHeader.pack(b"cut.xmlb", b"xml", len(document) + 1) + document
        try:
            list(iter_entries(truncated_fb))
            refused_truncated_fb = False
        except ValueError:
            refused_truncated_fb = True
        checks.append(("truncated FB payload refusal", refused_truncated_fb))

        broken_paths = dict(paths)
        broken_paths[REQUIRED_ARCHIVES[2]] = root / "broken.zip"
        _write_zip(broken_paths[REQUIRED_ARCHIVES[2]], {"data/dlc.xmlb": b"bad"})
        broken = inspect_archives(broken_paths, root, igb_dump=None)
        checks.append(("invalid XMLB negative", not broken.passed(require_igb=False)))

    for name, passed in checks:
        print(f"{'ok  ' if passed else 'FAIL'} {name}")
    passed = sum(result for _, result in checks)
    print(f"mua corpus selftest: {passed} of {len(checks)} discriminator checks passed")
    return 0 if passed == len(checks) else 1


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ps2-iso", type=Path, help="Marvel Ultimate Alliance PS2 ISO")
    parser.add_argument("--x360-iso", type=Path, help="MUA Gold Edition Xbox 360 XISO")
    parser.add_argument("--igb-dump", type=Path, help="shipping igb_dump executable")
    parser.add_argument("--scratch", type=Path, default=DEFAULT_SCRATCH)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    if args.selftest:
        return selftest(args.scratch)
    try:
        ps2_iso = _check_iso(args.ps2_iso, "PS2")
        x360_iso = _check_iso(args.x360_iso, "Xbox 360 Gold Edition")
        igb_dump = _find_igb_dump(args.igb_dump)
        args.scratch.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="run-", dir=args.scratch) as temp:
            work_root = Path(temp)
            archives = stage_archives(ps2_iso, x360_iso, work_root)
            stats = inspect_archives(archives, work_root, igb_dump)
    except CorpusRefused as error:
        print("archives: 0 of 4 required opened", file=sys.stderr)
        print(f"REFUSED: {error}", file=sys.stderr)
        return 2
    print_report(stats)
    if stats.passed():
        print("mua corpus: PASS")
        return 0
    print("mua corpus: FAIL")
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
