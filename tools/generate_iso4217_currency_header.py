#!/usr/bin/env python3
import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / 'data' / 'iso4217' / 'codes-all.csv'
OUT_PATH = ROOT / 'include' / 'secids' / 'iso4217_currency.hpp'


def cxx_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def pack_alpha3_compact(text: str) -> int:
    value = 0
    for ch in text:
        value = value * 26 + (ord(ch) - ord('A'))
    return value


with CSV_PATH.open(encoding='utf-8-sig', newline='') as f:
    rows = list(csv.DictReader(f))

current_rows = [
    r for r in rows
    if not r['WithdrawalDate'].strip()
    and r['AlphabeticCode'].strip()
    and r['NumericCode'].strip()
]
unique = {}
for row in current_rows:
    key = (
        row['AlphabeticCode'].strip(),
        row['NumericCode'].strip(),
        row['MinorUnit'].strip(),
        row['Currency'].strip(),
    )
    unique.setdefault(key, row)
entries = list(unique.values())
entries.sort(key=lambda r: (r['AlphabeticCode'].strip(), r['NumericCode'].strip(), r['Currency'].strip()))

alphabetic_to_row = [0xFFFF] * (26 * 26 * 26)
numeric_to_row = [0xFFFF] * 1000
for idx, row in enumerate(entries):
    alpha = row['AlphabeticCode'].strip()
    num = row['NumericCode'].strip()
    if alpha:
        alphabetic_to_row[pack_alpha3_compact(alpha)] = idx
    if num:
        numeric_to_row[int(num)] = idx

lines = []
lines.append('#ifndef SECIDS_ISO4217_CURRENCY_HPP')
lines.append('#define SECIDS_ISO4217_CURRENCY_HPP')
lines.append('')
lines.append('#include <array>')
lines.append('#include <cstdint>')
lines.append('#include <optional>')
lines.append('#include <string>')
lines.append('#include <string_view>')
lines.append('#include "secids/detail/alpha_code16.hpp"')
lines.append('')
lines.append('namespace secids::iso4217_currency {')
lines.append('')
lines.append('using alphabetic_packed_type = std::uint16_t;')
lines.append('using row_index_type = std::uint16_t;')
lines.append('inline constexpr std::uint16_t missing_code = 0xFFFFu;')
lines.append('inline constexpr std::uint8_t variable_minor_unit = 0xFEu;')
lines.append('inline constexpr std::uint8_t missing_minor_unit = 0xFFu;')
lines.append('')
lines.append('struct currency_entry {')
lines.append('    std::string_view alphabetic_code;')
lines.append('    alphabetic_packed_type alphabetic_compact;')
lines.append('    std::uint16_t numeric_code;')
lines.append('    std::uint8_t minor_unit;')
lines.append('    std::string_view currency;')
lines.append('};')
lines.append('')
lines.append('constexpr std::optional<alphabetic_packed_type> pack_alphabetic_code(std::string_view code) noexcept {')
lines.append('    return secids::detail::alpha_code16::pack<3>(code);')
lines.append('}')
lines.append('')
lines.append('inline std::string unpack_alphabetic_code(alphabetic_packed_type value) {')
lines.append('    return secids::detail::alpha_code16::unpack<3>(value);')
lines.append('}')
lines.append('')
lines.append('inline std::string format_numeric_code(std::uint16_t value) {')
lines.append('    if (value == missing_code) return {};')
lines.append('    std::string out(3, "0"[0]);')
lines.append('    out[0] = static_cast<char>("0"[0] + ((value / 100U) % 10U));')
lines.append('    out[1] = static_cast<char>("0"[0] + ((value / 10U) % 10U));')
lines.append('    out[2] = static_cast<char>("0"[0] + (value % 10U));')
lines.append('    return out;')
lines.append('}')
lines.append('')
lines.append('inline std::string minor_unit_to_string(std::uint8_t value) {')
lines.append('    if (value == missing_minor_unit) return {};')
lines.append('    if (value == variable_minor_unit) return "-";')
lines.append('    return std::to_string(value);')
lines.append('}')
lines.append('')
lines.append(f'inline constexpr std::array<currency_entry, {len(entries)}> currencies{{')
for row in entries:
    alpha = row['AlphabeticCode'].strip()
    num = int(row['NumericCode']) if row['NumericCode'].strip() else 0xFFFF
    minor_raw = row['MinorUnit'].strip()
    if minor_raw == '-':
        minor = 'variable_minor_unit'
    elif minor_raw == '':
        minor = 'missing_minor_unit'
    else:
        minor = f'{int(minor_raw)}u'
    lines.append('    currency_entry{')
    lines.append(f'        {cxx_string(alpha)},')
    lines.append(f'        static_cast<alphabetic_packed_type>({pack_alpha3_compact(alpha)}u),')
    lines.append(f'        static_cast<std::uint16_t>({num}),')
    lines.append(f'        static_cast<std::uint8_t>({minor}),')
    lines.append(f'        {cxx_string(row["Currency"].strip())},')
    lines.append('    },')
lines.append('};')
lines.append('')
lines.append(f'inline constexpr std::array<row_index_type, {len(alphabetic_to_row)}> alphabetic_to_row{{')
for i, idx in enumerate(alphabetic_to_row):
    suffix = ',' if i < len(alphabetic_to_row) - 1 else ''
    value = 'missing_code' if idx == 0xFFFF else f'{idx}u'
    lines.append(f'    static_cast<row_index_type>({value}){suffix}')
lines.append('};')
lines.append('')
lines.append('inline constexpr std::array<row_index_type, 1000> numeric_to_row{')
for i, idx in enumerate(numeric_to_row):
    suffix = ',' if i < len(numeric_to_row) - 1 else ''
    value = 'missing_code' if idx == 0xFFFF else f'{idx}u'
    lines.append(f'    static_cast<row_index_type>({value}){suffix}')
lines.append('};')
lines.append('')
lines.append('constexpr const currency_entry* find_by_alphabetic_code(std::string_view code) noexcept {')
lines.append('    const auto packed = pack_alphabetic_code(code);')
lines.append('    if (!packed) return nullptr;')
lines.append('    const auto row = alphabetic_to_row[*packed];')
lines.append('    return row == missing_code ? nullptr : &currencies[row];')
lines.append('}')
lines.append('')
lines.append('constexpr const currency_entry* find_by_numeric_code(std::uint16_t code) noexcept {')
lines.append('    if (code >= numeric_to_row.size()) return nullptr;')
lines.append('    const auto row = numeric_to_row[code];')
lines.append('    return row == missing_code ? nullptr : &currencies[row];')
lines.append('}')
lines.append('')
lines.append(f'inline constexpr std::size_t currency_count = currencies.size();')
lines.append(f'inline constexpr std::size_t raw_source_row_count = {len(rows)};')
lines.append(f'inline constexpr std::size_t current_source_row_count = {len(current_rows)};')
lines.append('inline constexpr std::size_t current_unique_currency_count = currencies.size();')
lines.append('')
lines.append('} // namespace secids::iso4217_currency')
lines.append('')
lines.append('#endif')
OUT_PATH.write_text('\n'.join(lines) + '\n', encoding='utf-8')
print(f'Generated {OUT_PATH.relative_to(ROOT)} from {CSV_PATH.relative_to(ROOT)} with {len(entries)} current unique currencies.')
