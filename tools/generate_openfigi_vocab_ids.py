#!/usr/bin/env python3
import json
import re
import sys
from pathlib import Path

CPP_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool",
    "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class",
    "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast",
    "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
    "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
    "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
    "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
    "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
    "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
    "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}


def sanitize(name: str) -> str:
    text = name.strip().lower()
    text = re.sub(r"[^a-z0-9]+", "_", text).strip("_")
    if not text:
        text = "empty"
    if text[0].isdigit():
        text = "value_" + text
    if text in CPP_KEYWORDS:
        text += "_value"
    return text


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: generate_openfigi_vocab_ids.py <domain> <json-array> <out-header>", file=sys.stderr)
        return 1

    domain = sys.argv[1]
    values = json.loads(Path(sys.argv[2]).read_text(encoding="utf-8"))
    if isinstance(values, dict):
        values = values.get("values")
    if not isinstance(values, list) or not all(isinstance(value, str) for value in values):
        raise ValueError("input must be a JSON string array or an object containing a string-array 'values' field")
    normalized = sorted({value.strip() for value in values if value.strip()})

    namespace = sanitize(domain)
    guard = f"SECIDS_OPENFIGI_GENERATED_{namespace.upper()}_VOCAB_IDS_HPP"
    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <cstdint>",
        "",
        "namespace secids::openfigi::ids {",
        f"namespace {namespace} {{",
    ]
    used_names: set[str] = set()
    for index, value in enumerate(normalized):
        base_name = sanitize(value)
        name = base_name
        suffix = 2
        while name in used_names:
            name = f"{base_name}_{suffix}"
            suffix += 1
        used_names.add(name)
        lines.append(f"inline constexpr std::uint32_t {name} = {index}u;")
    lines += [
        f"}} // namespace {namespace}",
        "} // namespace secids::openfigi::ids",
        "",
        "#endif",
    ]

    output = Path(sys.argv[3])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
