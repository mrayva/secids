#!/usr/bin/env python3

import csv
import hashlib
import json
import re
from pathlib import Path


ROOT = Path(__file__).parents[1]
MANIFEST_PATH = ROOT / "data" / "iso_sources_manifest.json"
UTC_PATTERN = re.compile(r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$")


def count_rows(path: Path) -> int:
    if path.suffix == ".csv":
        with path.open(encoding="utf-8-sig", newline="") as source_file:
            return max(0, sum(1 for _ in csv.reader(source_file)) - 1)
    document = json.loads(path.read_text(encoding="utf-8"))
    assert isinstance(document, dict)
    return len(document)


def main() -> None:
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    assert manifest["schema_version"] == 1
    assert len(manifest["sources"]) == 3

    for source in manifest["sources"]:
        path = ROOT / source["path"]
        assert path.is_file()
        assert source["source_url"].startswith("https://")
        assert UTC_PATTERN.fullmatch(source["retrieved_at_utc"])
        assert source["sha256"] == hashlib.sha256(path.read_bytes()).hexdigest()
        assert source["row_count"] == count_rows(path)


if __name__ == "__main__":
    main()
