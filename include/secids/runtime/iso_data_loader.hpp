#ifndef SECIDS_RUNTIME_ISO_DATA_LOADER_HPP
#define SECIDS_RUNTIME_ISO_DATA_LOADER_HPP

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace secids::runtime {

struct iso3166_country_csv_row {
    std::string name;
    std::string alpha2;
    std::string alpha3;
    std::string country_code;
    std::string iso_3166_2;
    std::string region;
    std::string sub_region;
    std::string intermediate_region;
    std::string region_code;
    std::string sub_region_code;
    std::string intermediate_region_code;
};

struct iso4217_currency_csv_row {
    std::string entity;
    std::string currency;
    std::string alphabetic_code;
    std::string numeric_code;
    std::string minor_unit;
    std::string withdrawal_date;
};

struct iso4217_currency_json_row {
    std::string alphabetic_code;
    std::string currency;
    std::optional<std::uint16_t> numeric_code;
    std::optional<std::uint8_t> minor_unit;
    std::string demonym;
    std::string symbol;
    std::string symbol_native;
    std::string major_single;
    std::string major_plural;
    std::string minor_single;
    std::string minor_plural;
};

namespace detail {

inline std::string trim(std::string value) {
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])) != 0) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])) != 0) {
        --last;
    }

    return value.substr(first, last - first);
}

inline std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open file: " + path.string());
    }

    std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content.erase(0, 3);
    }

    return content;
}

inline std::vector<std::string> parse_csv_line(std::string_view line) {
    std::vector<std::string> fields;
    std::string current;
    bool in_quotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (in_quotes) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current.push_back('"');
                    ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                current.push_back(ch);
            }
            continue;
        }

        if (ch == '"') {
            in_quotes = true;
        } else if (ch == ',') {
            fields.push_back(std::move(current));
            current.clear();
        } else {
            current.push_back(ch);
        }
    }

    if (in_quotes) {
        throw std::runtime_error("unterminated CSV quoted field");
    }

    fields.push_back(std::move(current));
    return fields;
}

inline std::vector<std::vector<std::string>> parse_csv_rows(const std::filesystem::path& path) {
    const std::string content = read_file(path);

    std::vector<std::vector<std::string>> rows;
    std::string current_line;
    bool in_quotes = false;

    for (std::size_t i = 0; i < content.size(); ++i) {
        const char ch = content[i];
        if (ch == '"') {
            current_line.push_back(ch);
            if (in_quotes && i + 1 < content.size() && content[i + 1] == '"') {
                current_line.push_back(content[++i]);
            } else {
                in_quotes = !in_quotes;
            }
            continue;
        }

        if (!in_quotes && (ch == '\n' || ch == '\r')) {
            if (ch == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
                ++i;
            }
            if (!current_line.empty()) {
                rows.push_back(parse_csv_line(current_line));
                current_line.clear();
            }
            continue;
        }

        current_line.push_back(ch);
    }

    if (in_quotes) {
        throw std::runtime_error("unterminated CSV quoted record");
    }

    if (!current_line.empty()) {
        rows.push_back(parse_csv_line(current_line));
    }

    return rows;
}

inline void append_utf8(std::string& out, std::uint32_t code_point) {
    if (code_point <= 0x7FU) {
        out.push_back(static_cast<char>(code_point));
    } else if (code_point <= 0x7FFU) {
        out.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else if (code_point <= 0xFFFFU) {
        out.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else {
        out.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    }
}

class json_reader {
public:
    explicit json_reader(std::string_view input) : input_(input) {}

    void skip_ws() {
        while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_])) != 0) {
            ++pos_;
        }
    }

    char peek() {
        skip_ws();
        if (pos_ >= input_.size()) {
            throw std::runtime_error("unexpected end of JSON input");
        }
        return input_[pos_];
    }

    bool consume(char ch) {
        skip_ws();
        if (pos_ < input_.size() && input_[pos_] == ch) {
            ++pos_;
            return true;
        }
        return false;
    }

    void expect(char ch) {
        if (!consume(ch)) {
            throw std::runtime_error(std::string("expected JSON character: ") + ch);
        }
    }

    std::string parse_string() {
        skip_ws();
        expect('"');
        std::string out;
        while (pos_ < input_.size()) {
            const char ch = input_[pos_++];
            if (ch == '"') {
                return out;
            }
            if (ch == '\\') {
                if (pos_ >= input_.size()) {
                    throw std::runtime_error("invalid JSON escape");
                }
                const char esc = input_[pos_++];
                switch (esc) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u': {
                        if (pos_ + 4 > input_.size()) {
                            throw std::runtime_error("invalid JSON unicode escape");
                        }
                        std::uint32_t code_point = 0;
                        for (int i = 0; i < 4; ++i) {
                            const char hex = input_[pos_++];
                            code_point <<= 4U;
                            if (hex >= '0' && hex <= '9') {
                                code_point |= static_cast<std::uint32_t>(hex - '0');
                            } else if (hex >= 'A' && hex <= 'F') {
                                code_point |= static_cast<std::uint32_t>(hex - 'A' + 10);
                            } else if (hex >= 'a' && hex <= 'f') {
                                code_point |= static_cast<std::uint32_t>(hex - 'a' + 10);
                            } else {
                                throw std::runtime_error("invalid JSON unicode escape");
                            }
                        }
                        append_utf8(out, code_point);
                        break;
                    }
                    default:
                        throw std::runtime_error("invalid JSON escape");
                }
            } else {
                out.push_back(ch);
            }
        }
        throw std::runtime_error("unterminated JSON string");
    }

    std::optional<std::int64_t> parse_nullable_integer() {
        skip_ws();
        if (match_literal("null")) {
            return std::nullopt;
        }

        bool negative = false;
        if (pos_ < input_.size() && input_[pos_] == '-') {
            negative = true;
            ++pos_;
        }

        if (pos_ >= input_.size() || !std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            throw std::runtime_error("expected JSON integer");
        }

        std::int64_t value = 0;
        while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            value = value * 10 + (input_[pos_] - '0');
            ++pos_;
        }

        return negative ? -value : value;
    }

    void skip_value() {
        skip_ws();
        const char ch = peek();
        if (ch == '{') {
            expect('{');
            if (consume('}')) {
                return;
            }
            do {
                parse_string();
                expect(':');
                skip_value();
            } while (consume(','));
            expect('}');
            return;
        }

        if (ch == '[') {
            expect('[');
            if (consume(']')) {
                return;
            }
            do {
                skip_value();
            } while (consume(','));
            expect(']');
            return;
        }

        if (ch == '"') {
            parse_string();
            return;
        }

        if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-') {
            parse_nullable_integer();
            return;
        }

        if (match_literal("true") || match_literal("false") || match_literal("null")) {
            return;
        }

        throw std::runtime_error("unsupported JSON value");
    }

private:
    bool match_literal(const char* literal) {
        const std::size_t start = pos_;
        while (*literal != '\0') {
            if (pos_ >= input_.size() || input_[pos_] != *literal) {
                pos_ = start;
                return false;
            }
            ++pos_;
            ++literal;
        }
        return true;
    }

    std::string_view input_;
    std::size_t pos_ = 0;
};

inline std::unordered_map<std::string, std::string> parse_json_string_map(json_reader& reader) {
    std::unordered_map<std::string, std::string> result;
    reader.expect('{');
    if (reader.consume('}')) {
        return result;
    }

    do {
        const std::string key = reader.parse_string();
        reader.expect(':');
        result.emplace(key, reader.parse_string());
    } while (reader.consume(','));

    reader.expect('}');
    return result;
}

inline std::unordered_map<std::string, std::optional<std::int64_t>> parse_json_nullable_integer_map(json_reader& reader) {
    std::unordered_map<std::string, std::optional<std::int64_t>> result;
    reader.expect('{');
    if (reader.consume('}')) {
        return result;
    }

    do {
        const std::string key = reader.parse_string();
        reader.expect(':');
        result.emplace(key, reader.parse_nullable_integer());
    } while (reader.consume(','));

    reader.expect('}');
    return result;
}

} // namespace detail

inline std::vector<iso3166_country_csv_row> load_iso3166_country_csv(const std::filesystem::path& path) {
    const auto rows = detail::parse_csv_rows(path);
    if (rows.empty()) {
        return {};
    }

    const std::vector<std::string>& header = rows.front();
    if (header.size() != 11 ||
        header[0] != "name" ||
        header[1] != "alpha-2" ||
        header[2] != "alpha-3" ||
        header[3] != "country-code") {
        throw std::runtime_error("unexpected ISO-3166 CSV header");
    }

    std::vector<iso3166_country_csv_row> out;
    out.reserve(rows.size() - 1);
    for (std::size_t i = 1; i < rows.size(); ++i) {
        const auto& row = rows[i];
        if (row.size() != 11) {
            throw std::runtime_error("unexpected ISO-3166 CSV row width");
        }
        out.push_back(iso3166_country_csv_row{
            row[0], row[1], row[2], row[3], row[4], row[5],
            row[6], row[7], row[8], row[9], row[10]
        });
    }
    return out;
}

inline std::vector<iso4217_currency_csv_row> load_iso4217_currency_csv(const std::filesystem::path& path) {
    const auto rows = detail::parse_csv_rows(path);
    if (rows.empty()) {
        return {};
    }

    const std::vector<std::string>& header = rows.front();
    if (header.size() != 6 ||
        header[0] != "Entity" ||
        header[1] != "Currency" ||
        header[2] != "AlphabeticCode" ||
        header[3] != "NumericCode") {
        throw std::runtime_error("unexpected ISO-4217 CSV header");
    }

    std::vector<iso4217_currency_csv_row> out;
    out.reserve(rows.size() - 1);
    for (std::size_t i = 1; i < rows.size(); ++i) {
        const auto& row = rows[i];
        if (row.size() != 6) {
            throw std::runtime_error("unexpected ISO-4217 CSV row width");
        }
        out.push_back(iso4217_currency_csv_row{
            row[0], row[1], row[2], row[3], row[4], row[5]
        });
    }
    return out;
}

inline std::vector<iso4217_currency_json_row> load_iso4217_currency_json(const std::filesystem::path& path) {
    const std::string content = detail::read_file(path);
    detail::json_reader reader(content);

    std::vector<iso4217_currency_json_row> out;

    reader.expect('{');
    if (reader.consume('}')) {
        return out;
    }

    do {
        iso4217_currency_json_row row;
        row.alphabetic_code = reader.parse_string();

        reader.expect(':');
        reader.expect('{');

        bool first_field = true;
        while (!reader.consume('}')) {
            if (!first_field) {
                reader.expect(',');
            }
            first_field = false;

            const std::string key = reader.parse_string();
            reader.expect(':');

            if (key == "name") {
                row.currency = reader.parse_string();
            } else if (key == "demonym") {
                row.demonym = reader.parse_string();
            } else if (key == "symbol") {
                row.symbol = reader.parse_string();
            } else if (key == "symbolNative") {
                row.symbol_native = reader.parse_string();
            } else if (key == "majorSingle") {
                row.major_single = reader.parse_string();
            } else if (key == "majorPlural") {
                row.major_plural = reader.parse_string();
            } else if (key == "minorSingle") {
                row.minor_single = reader.parse_string();
            } else if (key == "minorPlural") {
                row.minor_plural = reader.parse_string();
            } else if (key == "ISOnum") {
                const auto value = reader.parse_nullable_integer();
                if (value) {
                    row.numeric_code = static_cast<std::uint16_t>(*value);
                }
            } else if (key == "ISOdigits" || key == "decimals") {
                const auto value = reader.parse_nullable_integer();
                if (value && *value >= 0 && *value <= 255) {
                    row.minor_unit = static_cast<std::uint8_t>(*value);
                }
            } else {
                reader.skip_value();
            }
        }

        out.push_back(std::move(row));
    } while (reader.consume(','));

    reader.expect('}');
    return out;
}

} // namespace secids::runtime

#endif
