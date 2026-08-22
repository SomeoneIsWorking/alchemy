#!/usr/bin/env python3
"""Reusable readers for the ZIP-derived WAD and FB containers used by Alchemy."""

from __future__ import annotations

import shutil
import struct
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import BinaryIO, Self

_EOCD = struct.Struct("<4s4H2LH")
_EOCD_MAGIC = b"PK\x05\x06"
_MAX_EOCD_SEARCH = 65_557
_RAVEN_FORMATS = Path(__file__).parent / "raven-formats" / "src"


class ArchiveError(ValueError):
    """The input is absent or structurally unsafe to inspect."""


class _LimitedFile:
    """A seekable view ending at a selected EOCD, without copying the WAD."""

    def __init__(self, path: Path, limit: int):
        self._file = path.open("rb")
        self._limit = limit

    def read(self, size: int = -1) -> bytes:
        remaining = self._limit - self._file.tell()
        if size < 0 or size > remaining:
            size = remaining
        return self._file.read(size)

    def seek(self, offset: int, whence: int = 0) -> int:
        if whence == 0:
            target = offset
        elif whence == 1:
            target = self._file.tell() + offset
        elif whence == 2:
            target = self._limit + offset
        else:
            raise ValueError(f"invalid whence {whence}")
        if target < 0 or target > self._limit:
            raise ValueError(f"seek {target} is outside archive extent 0..{self._limit}")
        return self._file.seek(target)

    def tell(self) -> int:
        return self._file.tell()

    def seekable(self) -> bool:
        return True

    def readable(self) -> bool:
        return True

    def close(self) -> None:
        self._file.close()


def _zip_extent(path: Path) -> int:
    """Return the end of the EOCD with the largest valid central directory.

    PS2 ``ASSETSFB.WAD`` ends with a second, truncated EOCD signature.  Generic
    ZIP readers select that last signature and reject the real archive.  The
    archive's complete EOCD is still present immediately before it, so select
    among structurally complete records and prefer the one naming most files.
    """
    size = path.stat().st_size
    start = max(0, size - _MAX_EOCD_SEARCH)
    with path.open("rb") as source:
        source.seek(start)
        tail = source.read()
    candidates: list[tuple[int, int]] = []
    offset = 0
    while True:
        offset = tail.find(_EOCD_MAGIC, offset)
        if offset < 0:
            break
        if offset + _EOCD.size <= len(tail):
            record = _EOCD.unpack_from(tail, offset)
            _, disk, cd_disk, disk_entries, entries, cd_size, cd_offset, comment = record
            absolute = start + offset
            end = absolute + _EOCD.size + comment
            if (
                disk == 0
                and cd_disk == 0
                and disk_entries == entries
                and end <= size
                and cd_offset + cd_size <= absolute
            ):
                candidates.append((entries, end))
        offset += len(_EOCD_MAGIC)
    if not candidates:
        raise ArchiveError(f"{path}: no complete single-disk ZIP directory")
    return max(candidates)[1]


def _safe_member_path(name: str) -> PurePosixPath:
    path = PurePosixPath(name.replace("\\", "/"))
    if path.is_absolute() or not path.parts or ".." in path.parts:
        raise ArchiveError(f"unsafe archive member path {name!r}")
    return path


@dataclass(frozen=True)
class ArchiveMember:
    name: str
    size: int


class AlchemyZip:
    """Read standard ZIPs and Raven WADs through one validated interface."""

    def __init__(self, path: Path):
        self.path = path
        if not path.is_file():
            raise ArchiveError(f"archive is missing: {path}")
        self._view = _LimitedFile(path, _zip_extent(path))
        try:
            self._zip = zipfile.ZipFile(self._view)
            infos = self._zip.infolist()
        except (OSError, zipfile.BadZipFile) as error:
            self._view.close()
            raise ArchiveError(f"cannot open {path}: {error}") from error
        self._infos = {}
        try:
            for info in infos:
                if info.is_dir():
                    continue
                safe = _safe_member_path(info.filename)
                key = str(safe).casefold()
                if key in self._infos:
                    raise ArchiveError(f"{path}: duplicate member path {safe}")
                self._infos[key] = info
        except Exception:
            self._zip.close()
            self._view.close()
            raise

    def close(self) -> None:
        self._zip.close()
        self._view.close()

    def __enter__(self) -> Self:
        return self

    def __exit__(self, *_args: object) -> None:
        self.close()

    def members(self) -> list[ArchiveMember]:
        return [ArchiveMember(info.filename, info.file_size) for info in self._infos.values()]

    def open(self, name: str) -> BinaryIO:
        info = self._infos.get(name.replace("\\", "/").casefold())
        if info is None:
            raise ArchiveError(f"{self.path}: archive member is missing: {name}")
        return self._zip.open(info)

    def materialize(self, name: str, destination: Path) -> None:
        destination.parent.mkdir(parents=True, exist_ok=True)
        with self.open(name) as source, destination.open("wb") as output:
            shutil.copyfileobj(source, output)

    def extract(self, destination: Path, wanted: list[str] | None = None) -> int:
        extracted = 0
        lowered = [item.casefold() for item in wanted or []]
        for member in self.members():
            if lowered and not any(item in member.name.casefold() for item in lowered):
                continue
            target = destination.joinpath(*_safe_member_path(member.name).parts)
            self.materialize(member.name, target)
            extracted += 1
        return extracted


@dataclass(frozen=True)
class FbMember:
    name: str
    kind: str
    path: Path
    size: int


def extract_fb(package: Path, workspace: Path) -> list[FbMember]:
    """Run raven-formats' shipping FB parser and retain every occurrence."""
    raven_path = str(_RAVEN_FORMATS)
    if raven_path not in sys.path:
        sys.path.insert(0, raven_path)
    from raven_formats import fb

    members_root = workspace / "members"
    members_root.mkdir(parents=True, exist_ok=True)
    members = []
    for index, entry in enumerate(fb.iter_entries(package.read_bytes())):
        safe = _safe_member_path(entry.file_path)
        suffix = safe.suffix or ".bin"
        path = members_root / f"{index:05d}{suffix}"
        path.write_bytes(entry.data)
        members.append(FbMember(entry.file_path, entry.file_type, path, len(entry.data)))
    return members
