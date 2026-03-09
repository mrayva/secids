#ifndef SECIDS_CUSIP64_HPP
#define SECIDS_CUSIP64_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::cusip64 {

using value_type = std::uint64_t;
using decoded_type = std::array<char, 9>;

namespace detail {

constexpr value_type pow_u64(value_type base, unsigned exp) noexcept {
    value_type result = 1;
    for (unsigned i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

constexpr value_type base10_1 = 10;
constexpr value_type base39_1 = 39;
constexpr value_type base39_8 = pow_u64(base39_1, 8);
constexpr value_type max_encoded_value = base39_8 * base10_1 - 1;

constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

constexpr bool is_upper_alpha(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

constexpr char to_upper_ascii(char c) noexcept {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

constexpr std::optional<value_type> encode_base39(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return static_cast<value_type>(c - '0');
    }
    if (is_upper_alpha(c)) {
        return static_cast<value_type>(10 + (c - 'A'));
    }
    if (c == '*') {
        return 36;
    }
    if (c == '@') {
        return 37;
    }
    if (c == '#') {
        return 38;
    }
    return std::nullopt;
}

constexpr char decode_base39(value_type v) noexcept {
    if (v < 10) {
        return static_cast<char>('0' + v);
    }
    if (v < 36) {
        return static_cast<char>('A' + (v - 10));
    }
    if (v == 36) {
        return '*';
    }
    if (v == 37) {
        return '@';
    }
    return '#';
}

constexpr std::optional<int> checksum_value(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return c - '0';
    }
    if (is_upper_alpha(c)) {
        return 10 + (c - 'A');
    }
    if (c == '*') {
        return 36;
    }
    if (c == '@') {
        return 37;
    }
    if (c == '#') {
        return 38;
    }
    return std::nullopt;
}

} // namespace detail

inline constexpr value_type max_value = detail::max_encoded_value;

constexpr bool is_valid_cusip_format(std::string_view cusip) noexcept {
    if (cusip.size() != 9) {
        return false;
    }

    for (std::size_t i = 0; i < 8; ++i) {
        if (!detail::encode_base39(cusip[i]).has_value()) {
            return false;
        }
    }

    return detail::is_digit(cusip[8]);
}

constexpr std::optional<int> calculate_check_digit(std::string_view cusip_without_check_digit) noexcept {
    if (cusip_without_check_digit.size() != 8) {
        return std::nullopt;
    }

    int sum = 0;
    for (std::size_t i = 0; i < cusip_without_check_digit.size(); ++i) {
        const auto raw = detail::checksum_value(cusip_without_check_digit[i]);
        if (!raw.has_value()) {
            return std::nullopt;
        }

        int value = *raw;
        if ((i % 2U) == 1U) {
            value *= 2;
        }
        sum += (value / 10) + (value % 10);
    }

    return (10 - (sum % 10)) % 10;
}

constexpr bool has_valid_check_digit(std::string_view cusip) noexcept {
    if (!is_valid_cusip_format(cusip)) {
        return false;
    }

    const auto expected = calculate_check_digit(cusip.substr(0, 8));
    return expected.has_value() && *expected == (cusip[8] - '0');
}

constexpr bool is_valid_cusip(std::string_view cusip) noexcept {
    return has_valid_check_digit(cusip);
}

constexpr std::optional<value_type> encode_cusip(std::string_view cusip) noexcept {
    if (!is_valid_cusip_format(cusip)) {
        return std::nullopt;
    }

    value_type value = 0;
    for (std::size_t i = 0; i < 8; ++i) {
        value = value * 39 + *detail::encode_base39(cusip[i]);
    }
    value = value * 10 + static_cast<value_type>(cusip[8] - '0');
    return value;
}

constexpr std::optional<value_type> encode_valid_cusip(std::string_view cusip) noexcept {
    if (!is_valid_cusip(cusip)) {
        return std::nullopt;
    }
    return encode_cusip(cusip);
}

constexpr std::optional<decoded_type> decode_cusip(value_type value) noexcept {
    if (value > max_value) {
        return std::nullopt;
    }

    decoded_type out{};
    out[8] = static_cast<char>('0' + (value % 10));
    value /= 10;

    for (int i = 7; i >= 0; --i) {
        out[static_cast<std::size_t>(i)] = detail::decode_base39(value % 39);
        value /= 39;
    }

    return out;
}

inline std::string to_string(const decoded_type& cusip) {
    return std::string(cusip.begin(), cusip.end());
}

} // namespace secids::cusip64

#endif
