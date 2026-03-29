#!/usr/bin/env python3
import json
import re
import sys
from pathlib import Path


def sanitize(name: str) -> str:
    text = name.strip().lower()
    text = re.sub(r"[^a-z0-9]+", "_", text).strip("_")
    if not text:
        text = "empty"
    if text[0].isdigit():
        text = "_" + text
    return text


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: generate_openfigi_vocab_ids.py <domain> <json-array> <out-header>", file=sys.stderr)
        return 1

    domain = sys.argv[1]
    values = json.loads(Path(sys.argv[2]).read_text(encoding="utf-8"))
    if isinstance(values, dict):
        values = values["values"]
    normalized = sorted({value.strip() for value in values if value.strip()})

    namespace = sanitize(domain)
    lines = [
        "#ifndef SECIDS_OPENFIGI_GENERATED_VOCAB_IDS_HPP",
        "#define SECIDS_OPENFIGI_GENERATED_VOCAB_IDS_HPP",
        "",
        "#include <cstdint>",
        "",
        "namespace secids::openfigi::ids {",
        f"namespace {namespace} {{",
    ]
    for index, value in enumerate(normalized):
        lines.append(f"inline constexpr std::uint32_t {sanitize(value)} = {index}u;")
    lines += [
        f"}} // namespace {namespace}",
        "} // namespace secids::openfigi::ids",
        "",
        "#endif",
    ]

    Path(sys.argv[3]).write_text("\n".join(lines) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
