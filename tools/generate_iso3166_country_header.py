#!/usr/bin/env python3
import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / 'data' / 'iso3166' / 'all.csv'
OUT_PATH = ROOT / 'include' / 'secids' / 'iso3166_country.hpp'


def pack_alpha3_compact(text: str) -> int:
    value = 0
    for ch in text:
        value = value * 26 + (ord(ch) - ord('A'))
    return value


def pack_alpha2_compact(text: str) -> int:
    value = 0
    for ch in text:
        value = value * 26 + (ord(ch) - ord('A'))
    return value


def enum_name(label: str) -> str:
    if not label:
        return 'unspecified'
    out = []
    prev_underscore = False
    for ch in label.lower():
        if ch.isalnum():
            out.append(ch)
            prev_underscore = False
        else:
            if not prev_underscore:
                out.append('_')
                prev_underscore = True
    name = ''.join(out).strip('_')
    if not name:
        return 'unspecified'
    if name[0].isdigit():
        name = '_' + name
    return name


def cxx_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


with CSV_PATH.open(encoding='utf-8-sig', newline='') as f:
    rows = list(csv.DictReader(f))

regions = ['']
sub_regions = ['']
intermediate_regions = ['']
for row in rows:
    if row['region'] not in regions:
        regions.append(row['region'])
    if row['sub-region'] not in sub_regions:
        sub_regions.append(row['sub-region'])
    if row['intermediate-region'] not in intermediate_regions:
        intermediate_regions.append(row['intermediate-region'])

region_ids = {label: idx for idx, label in enumerate(regions)}
sub_region_ids = {label: idx for idx, label in enumerate(sub_regions)}
intermediate_region_ids = {label: idx for idx, label in enumerate(intermediate_regions)}

country_code_to_row = [0xFFFF] * 1000
alpha2_to_row = [0xFFFF] * (26 * 26)
alpha3_to_row = [0xFFFF] * (26 * 26 * 26)
for idx, row in enumerate(rows):
    country_code_to_row[int(row['country-code'])] = idx
    alpha2_to_row[pack_alpha2_compact(row['alpha-2'])] = idx
    alpha3_to_row[pack_alpha3_compact(row['alpha-3'])] = idx

lines = []
lines.append('#ifndef SECIDS_ISO3166_COUNTRY_HPP')
lines.append('#define SECIDS_ISO3166_COUNTRY_HPP')
lines.append('')
lines.append('#include <array>')
lines.append('#include <cstdint>')
lines.append('#include <optional>')
lines.append('#include <string>')
lines.append('#include <string_view>')
lines.append('#include "secids/detail/alpha_code16.hpp"')
lines.append('')
lines.append('namespace secids::iso3166_country {')
lines.append('')
lines.append('using alpha2_packed_type = std::uint16_t;')
lines.append('using alpha3_packed_type = std::uint16_t;')
lines.append('using row_index_type = std::uint16_t;')
lines.append('inline constexpr std::uint16_t missing_code = 0xFFFFu;')
lines.append('')
lines.append('enum class region : std::uint8_t {')
for label in regions:
    lines.append(f'    {enum_name(label)} = {region_ids[label]},')
lines.append('};')
lines.append('')
lines.append('enum class sub_region : std::uint8_t {')
for label in sub_regions:
    lines.append(f'    {enum_name(label)} = {sub_region_ids[label]},')
lines.append('};')
lines.append('')
lines.append('enum class intermediate_region : std::uint8_t {')
for label in intermediate_regions:
    lines.append(f'    {enum_name(label)} = {intermediate_region_ids[label]},')
lines.append('};')
lines.append('')
lines.append('struct country_entry {')
lines.append('    std::string_view name;')
lines.append('    alpha2_packed_type alpha2;')
lines.append('    alpha3_packed_type alpha3;')
lines.append('    std::uint16_t country_code;')
lines.append('    region region_id;')
lines.append('    sub_region sub_region_id;')
lines.append('    intermediate_region intermediate_region_id;')
lines.append('    std::uint16_t region_code;')
lines.append('    std::uint16_t sub_region_code;')
lines.append('    std::uint16_t intermediate_region_code;')
lines.append('};')
lines.append('')
lines.append('constexpr std::optional<alpha2_packed_type> pack_alpha2(std::string_view code) noexcept {')
lines.append('    return secids::detail::alpha_code16::pack<2>(code);')
lines.append('}')
lines.append('')
lines.append('constexpr std::optional<row_index_type> pack_alpha2_compact(std::string_view code) noexcept {')
lines.append('    return secids::detail::alpha_code16::pack<2>(code);')
lines.append('}')
lines.append('')
lines.append('constexpr std::optional<alpha3_packed_type> pack_alpha3(std::string_view code) noexcept {')
lines.append('    return secids::detail::alpha_code16::pack<3>(code);')
lines.append('}')
lines.append('')
lines.append('inline std::string unpack_alpha2(alpha2_packed_type value) {')
lines.append('    return secids::detail::alpha_code16::unpack<2>(value);')
lines.append('}')
lines.append('')
lines.append('inline std::string unpack_alpha3(alpha3_packed_type value) {')
lines.append('    return secids::detail::alpha_code16::unpack<3>(value);')
lines.append('}')
lines.append('')
lines.append('inline std::string format_numeric_code(std::uint16_t value) {')
lines.append('    if (value == missing_code) return {};')
lines.append('    std::string out(3, \"0\"[0]);')
lines.append('    out[0] = static_cast<char>(\"0\"[0] + ((value / 100U) % 10U));')
lines.append('    out[1] = static_cast<char>(\"0\"[0] + ((value / 10U) % 10U));')
lines.append('    out[2] = static_cast<char>(\"0\"[0] + (value % 10U));')
lines.append('    return out;')
lines.append('}')
lines.append('')
for arr_name, labels, enum_type in [
    ('region_names', regions, 'region'),
    ('sub_region_names', sub_regions, 'sub_region'),
    ('intermediate_region_names', intermediate_regions, 'intermediate_region'),
]:
    lines.append(f'inline constexpr std::array<std::string_view, {len(labels)}> {arr_name}{{')
    for label in labels:
        lines.append(f'    {cxx_string(label)},')
    lines.append('};')
    lines.append('')
    lines.append(f'constexpr std::string_view to_string({enum_type} value) noexcept {{')
    lines.append(f'    return {arr_name}[static_cast<std::size_t>(value)];')
    lines.append('}')
    lines.append('')

lines.append(f'inline constexpr std::array<country_entry, {len(rows)}> countries{{')
for row in rows:
    alpha2 = pack_alpha2_compact(row['alpha-2'])
    alpha3 = pack_alpha3_compact(row['alpha-3'])
    country_code = int(row['country-code'])
    region_code = int(row['region-code']) if row['region-code'] else 0xFFFF
    sub_region_code = int(row['sub-region-code']) if row['sub-region-code'] else 0xFFFF
    intermediate_region_code = int(row['intermediate-region-code']) if row['intermediate-region-code'] else 0xFFFF
    lines.append('    country_entry{')
    lines.append(f'        {cxx_string(row["name"])},')
    lines.append(f'        static_cast<alpha2_packed_type>({alpha2}u),')
    lines.append(f'        static_cast<alpha3_packed_type>({alpha3}u),')
    lines.append(f'        static_cast<std::uint16_t>({country_code}),')
    lines.append(f'        region::{enum_name(row["region"] )},')
    lines.append(f'        sub_region::{enum_name(row["sub-region"] )},')
    lines.append(f'        intermediate_region::{enum_name(row["intermediate-region"] )},')
    lines.append(f'        static_cast<std::uint16_t>({region_code if region_code != 0xFFFF else "missing_code"}),')
    lines.append(f'        static_cast<std::uint16_t>({sub_region_code if sub_region_code != 0xFFFF else "missing_code"}),')
    lines.append(f'        static_cast<std::uint16_t>({intermediate_region_code if intermediate_region_code != 0xFFFF else "missing_code"}),')
    lines.append('    },')
lines.append('};')
lines.append('')
lines.append('inline constexpr std::array<row_index_type, 1000> country_code_to_row{')
for i, idx in enumerate(country_code_to_row):
    suffix = ',' if i < len(country_code_to_row) - 1 else ''
    value = 'missing_code' if idx == 0xFFFF else f'{idx}u'
    lines.append(f'    static_cast<row_index_type>({value}){suffix}')
lines.append('};')
lines.append('')
lines.append(f'inline constexpr std::array<row_index_type, {len(alpha2_to_row)}> alpha2_to_row{{')
for i, idx in enumerate(alpha2_to_row):
    suffix = ',' if i < len(alpha2_to_row) - 1 else ''
    value = 'missing_code' if idx == 0xFFFF else f'{idx}u'
    lines.append(f'    static_cast<row_index_type>({value}){suffix}')
lines.append('};')
lines.append('')
lines.append(f'inline constexpr std::array<row_index_type, {len(alpha3_to_row)}> alpha3_to_row{{')
for i, idx in enumerate(alpha3_to_row):
    suffix = ',' if i < len(alpha3_to_row) - 1 else ''
    value = 'missing_code' if idx == 0xFFFF else f'{idx}u'
    lines.append(f'    static_cast<row_index_type>({value}){suffix}')
lines.append('};')
lines.append('')
lines.append('constexpr const country_entry* find_by_alpha2(std::string_view code) noexcept {')
lines.append('    const auto compact = pack_alpha2_compact(code);')
lines.append('    if (!compact) return nullptr;')
lines.append('    const auto row = alpha2_to_row[*compact];')
lines.append('    return row == missing_code ? nullptr : &countries[row];')
lines.append('}')
lines.append('')
lines.append('constexpr const country_entry* find_by_alpha3(std::string_view code) noexcept {')
lines.append('    const auto packed = pack_alpha3(code);')
lines.append('    if (!packed) return nullptr;')
lines.append('    const auto row = alpha3_to_row[*packed];')
lines.append('    return row == missing_code ? nullptr : &countries[row];')
lines.append('}')
lines.append('')
lines.append('constexpr const country_entry* find_by_country_code(std::uint16_t code) noexcept {')
lines.append('    if (code >= country_code_to_row.size()) return nullptr;')
lines.append('    const auto row = country_code_to_row[code];')
lines.append('    return row == missing_code ? nullptr : &countries[row];')
lines.append('}')
lines.append('')
lines.append('inline std::string iso_3166_2_code(const country_entry& entry) {')
lines.append('    return std::string("ISO 3166-2:") + unpack_alpha2(entry.alpha2);')
lines.append('}')
lines.append('')
lines.append('inline constexpr std::size_t country_count = countries.size();')
lines.append('')
lines.append('} // namespace secids::iso3166_country')
lines.append('')
lines.append('#endif')
OUT_PATH.write_text('\n'.join(lines) + '\n', encoding='utf-8')
print(f'Generated {OUT_PATH.relative_to(ROOT)} from {CSV_PATH.relative_to(ROOT)} with {len(rows)} entries.')
