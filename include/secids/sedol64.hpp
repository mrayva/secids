#ifndef SECIDS_SEDOL64_HPP
#define SECIDS_SEDOL64_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::sedol64 {

using value_type = std::uint64_t;
using decoded_type = std::array<char, 7>;

namespace detail {

constexpr value_type pow_u64(value_type base, unsigned exp) noexcept {
    value_type result = 1;
    for (unsigned i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

constexpr value_type base10_1 = 10;
constexpr value_type base31_1 = 31;
constexpr value_type base31_6 = pow_u64(base31_1, 6);
constexpr value_type max_encoded_value = base31_6 * base10_1 - 1;

constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

constexpr bool is_upper_alpha(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

constexpr bool is_vowel(char c) noexcept {
    return c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U';
}

constexpr char to_upper_ascii(char c) noexcept {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

constexpr std::optional<value_type> encode_base31(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return static_cast<value_type>(c - '0');
    }
    if (!is_upper_alpha(c) || is_vowel(c)) {
        return std::nullopt;
    }

    constexpr char alphabet[] = "BCDFGHJKLMNPQRSTVWXYZ";
    for (value_type i = 0; i < 21; ++i) {
        if (alphabet[i] == c) {
            return 10 + i;
        }
    }
    return std::nullopt;
}

constexpr char decode_base31(value_type v) noexcept {
    if (v < 10) {
        return static_cast<char>('0' + v);
    }
    constexpr char alphabet[] = "BCDFGHJKLMNPQRSTVWXYZ";
    return alphabet[v - 10];
}

constexpr std::optional<int> checksum_value(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return c - '0';
    }
    if (is_upper_alpha(c)) {
        return 10 + (c - 'A');
    }
    return std::nullopt;
}

} // namespace detail

inline constexpr value_type max_value = detail::max_encoded_value;

constexpr bool is_valid_sedol_format(std::string_view sedol) noexcept {
    if (sedol.size() != 7) {
        return false;
    }

    for (std::size_t i = 0; i < 6; ++i) {
        if (!detail::encode_base31(sedol[i]).has_value()) {
            return false;
        }
    }

    return detail::is_digit(sedol[6]);
}

constexpr std::optional<int> calculate_check_digit(std::string_view sedol_without_check_digit) noexcept {
    if (sedol_without_check_digit.size() != 6) {
        return std::nullopt;
    }

    constexpr int weights[] = {1, 3, 1, 7, 3, 9};
    int sum = 0;
    for (std::size_t i = 0; i < 6; ++i) {
        const auto value = detail::checksum_value(sedol_without_check_digit[i]);
        if (!value.has_value()) {
            return std::nullopt;
        }
        if (!detail::encode_base31(sedol_without_check_digit[i]).has_value()) {
            return std::nullopt;
        }
        sum += weights[i] * *value;
    }

    return (10 - (sum % 10)) % 10;
}

constexpr bool has_valid_check_digit(std::string_view sedol) noexcept {
    if (!is_valid_sedol_format(sedol)) {
        return false;
    }

    const auto expected = calculate_check_digit(sedol.substr(0, 6));
    return expected.has_value() && *expected == (sedol[6] - '0');
}

constexpr bool is_valid_sedol(std::string_view sedol) noexcept {
    return has_valid_check_digit(sedol);
}

constexpr std::optional<value_type> encode_sedol(std::string_view sedol) noexcept {
    if (!is_valid_sedol_format(sedol)) {
        return std::nullopt;
    }

    value_type value = 0;
    for (std::size_t i = 0; i < 6; ++i) {
        value = value * 31 + *detail::encode_base31(sedol[i]);
    }
    value = value * 10 + static_cast<value_type>(sedol[6] - '0');
    return value;
}

constexpr std::optional<value_type> encode_valid_sedol(std::string_view sedol) noexcept {
    if (!is_valid_sedol(sedol)) {
        return std::nullopt;
    }
    return encode_sedol(sedol);
}

constexpr std::optional<decoded_type> decode_sedol(value_type value) noexcept {
    if (value > max_value) {
        return std::nullopt;
    }

    decoded_type out{};
    out[6] = static_cast<char>('0' + (value % 10));
    value /= 10;

    for (int i = 5; i >= 0; --i) {
        out[static_cast<std::size_t>(i)] = detail::decode_base31(value % 31);
        value /= 31;
    }

    return out;
}

inline std::string to_string(const decoded_type& sedol) {
    return std::string(sedol.begin(), sedol.end());
}

} // namespace secids::sedol64

#endif
