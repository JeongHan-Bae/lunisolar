#!/usr/bin/env python3
from __future__ import annotations

"""Generate the Shields endpoint JSON for the lunisolar version badge."""

import argparse
import json
import re
from pathlib import Path


DEFAULT_HEADER = Path("include/lunisolar.h")
DEFAULT_OUT = Path("version_badge.json")

VERSION_CONSTANT_RE = re.compile(
    r"\binline\s+constexpr\s+int\s+(major|minor|patch)\s*=\s*(\d+)\s*;"
)
META_NAMESPACE_RE = re.compile(r"\bnamespace\s+meta\s*\{(?P<body>.*?)\n\s*\}", re.DOTALL)


def read_meta_version(header_path: Path) -> tuple[int, int, int]:
    """Read lunisolar::meta version constants from the public header."""

    header = header_path.read_text(encoding="utf-8")
    for namespace_match in META_NAMESPACE_RE.finditer(header):
        constants = {
            name: int(value)
            for name, value in VERSION_CONSTANT_RE.findall(namespace_match.group("body"))
        }
        if {"major", "minor", "patch"} <= constants.keys():
            return constants["major"], constants["minor"], constants["patch"]

    raise ValueError(f"could not find meta major/minor/patch constants in {header_path}")


def build_badge(version: tuple[int, int, int], label: str) -> dict[str, object]:
    """Build a Shields endpoint response."""

    return {
        "schemaVersion": 1,
        "label": label,
        "message": "{}.{}.{}".format(*version),
        "labelColor": "#555555",
        "namedLogo": "github",
        "color": "#559900",
        "style": "flat",
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate version_badge.json from include/lunisolar.h metadata."
    )
    parser.add_argument(
        "--header",
        type=Path,
        default=DEFAULT_HEADER,
        help=f"header to read; default: {DEFAULT_HEADER}",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=DEFAULT_OUT,
        help=f"badge JSON output path; default: {DEFAULT_OUT}",
    )
    parser.add_argument(
        "--label",
        default="lunisolar",
        help="badge label; default: lunisolar",
    )
    parser.add_argument(
        "--version",
        action="store_true",
        help="print the project version instead of generating the badge JSON",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    version = read_meta_version(args.header)

    if not args.version:
        badge = build_badge(version, args.label)
        args.out.write_text(
            json.dumps(badge, indent=2) + "\n",
            encoding="utf-8",
            )
    else:
        print("{}.{}.{}".format(*version))


if __name__ == "__main__":
    main()
