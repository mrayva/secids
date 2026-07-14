#!/usr/bin/env python3

import json
import subprocess
import sys
import tempfile
from pathlib import Path


def generate(script: Path, root: Path, domain: str, values: list[str]) -> str:
    input_path = root / f"{domain}.json"
    output_path = root / f"{domain}.hpp"
    input_path.write_text(json.dumps({"values": values}), encoding="utf-8")
    subprocess.run(
        [sys.executable, str(script), domain, str(input_path), str(output_path)],
        check=True,
    )
    return output_path.read_text(encoding="utf-8")


def main() -> None:
    script = Path(__file__).parents[1] / "tools/generate_openfigi_vocab_ids.py"
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        first = generate(script, root, "micCode", ["123", "A-B", "A_B", "class", "XNYS"])
        second = generate(script, root, "exchCode", ["US"])

    assert "value_123 = 0" in first
    assert "a_b = 1" in first
    assert "a_b_2 = 2" in first
    assert "class_value = 4" in first
    assert "SECIDS_OPENFIGI_GENERATED_MICCODE_VOCAB_IDS_HPP" in first
    assert "SECIDS_OPENFIGI_GENERATED_EXCHCODE_VOCAB_IDS_HPP" in second
    assert "SECIDS_OPENFIGI_GENERATED_MICCODE_VOCAB_IDS_HPP" not in second


if __name__ == "__main__":
    main()
