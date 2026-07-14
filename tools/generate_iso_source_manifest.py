#!/usr/bin/env python3

import argparse
import csv
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
MANIFEST_PATH = ROOT / "data" / "iso_sources_manifest.json"
SOURCES = (
    {
        "dataset": "ISO 3166 countries with regional codes",
        "path": "data/iso3166/all.csv",
        "url": "https://raw.githubusercontent.com/lukes/ISO-3166-Countries-with-Regional-Codes/master/all/all.csv",
        "format": "csv",
    },
    {
        "dataset": "ISO 4217 currency codes",
        "path": "data/iso4217/codes-all.csv",
        "url": "https://raw.githubusercontent.com/datasets/currency-codes/main/data/codes-all.csv",
        "format": "csv",
    },
    {
        "dataset": "ISO 4217 currency metadata",
        "path": "data/iso4217-json/currencies.json",
        "url": "https://raw.githubusercontent.com/ourworldincode/currency/main/currencies.json",
        "format": "json-object",
    },
)


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def row_count(path: Path, source_format: str) -> int:
    if source_format == "csv":
        with path.open(encoding="utf-8-sig", newline="") as source_file:
            rows = sum(1 for _ in csv.reader(source_file))
        return max(0, rows - 1)

    document = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(document, dict):
        raise ValueError(f"expected a JSON object in {path}")
    return len(document)


def existing_retrievals() -> dict[tuple[str, str], str]:
    if not MANIFEST_PATH.exists():
        return {}
    document = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    return {
        (entry["path"], entry["sha256"]): entry["retrieved_at_utc"]
        for entry in document.get("sources", [])
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate ISO source provenance metadata")
    parser.add_argument(
        "--retrieved-at",
        help="UTC retrieval timestamp for all inputs; defaults to preserving timestamps for unchanged files",
    )
    args = parser.parse_args()

    previous = existing_retrievals()
    fallback_time = args.retrieved_at or utc_now()
    entries = []
    for source in SOURCES:
        path = ROOT / source["path"]
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        retrieved_at = args.retrieved_at or previous.get((source["path"], digest), fallback_time)
        entries.append(
            {
                "dataset": source["dataset"],
                "path": source["path"],
                "source_url": source["url"],
                "retrieved_at_utc": retrieved_at,
                "sha256": digest,
                "row_count": row_count(path, source["format"]),
            }
        )

    manifest = {"schema_version": 1, "sources": entries}
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Generated {MANIFEST_PATH.relative_to(ROOT)} with {len(entries)} sources.")


if __name__ == "__main__":
    main()
