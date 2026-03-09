#ifndef SECIDS_ISIN64_HPP
#define SECIDS_ISIN64_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::isin64 {

using value_type = std::uint64_t;
using decoded_type = std::array<char, 12>;

namespace detail {

constexpr value_type pow_u64(value_type base, unsigned exp) noexcept {
    value_type result = 1;
    for (unsigned i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

constexpr value_type base10_1 = 10;
constexpr value_type base36_1 = 36;
constexpr value_type base36_2 = pow_u64(36, 2);
constexpr value_type base36_3 = pow_u64(36, 3);
constexpr value_type base36_4 = pow_u64(36, 4);
constexpr value_type base36_5 = pow_u64(36, 5);
constexpr value_type base36_6 = pow_u64(36, 6);
constexpr value_type base36_7 = pow_u64(36, 7);
constexpr value_type base36_8 = pow_u64(36, 8);
constexpr value_type base36_9 = pow_u64(36, 9);
constexpr value_type base26_1 = 26;
constexpr value_type base26_2 = pow_u64(26, 2);

constexpr value_type max_encoded_value = base26_2 * base36_9 * base10_1 - 1;

constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

constexpr bool is_upper_alpha(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

constexpr char to_upper_ascii(char c) noexcept {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

constexpr std::optional<value_type> encode_letter(char c) noexcept {
    c = to_upper_ascii(c);
    if (!is_upper_alpha(c)) {
        return std::nullopt;
    }
    return static_cast<value_type>(c - 'A');
}

constexpr std::optional<value_type> encode_alnum(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return static_cast<value_type>(c - '0');
    }
    if (is_upper_alpha(c)) {
        return static_cast<value_type>(10 + (c - 'A'));
    }
    return std::nullopt;
}

constexpr char decode_letter(value_type v) noexcept {
    return static_cast<char>('A' + v);
}

constexpr char decode_alnum(value_type v) noexcept {
    return (v < 10)
        ? static_cast<char>('0' + v)
        : static_cast<char>('A' + (v - 10));
}

constexpr int digit_sum_for_isin_char(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return c - '0';
    }
    if (is_upper_alpha(c)) {
        const int value = 10 + (c - 'A');
        return (value / 10) + (value % 10);
    }
    return -1;
}

} // namespace detail

inline constexpr value_type max_value = detail::max_encoded_value;

constexpr bool is_valid_isin_format(std::string_view isin) noexcept {
    if (isin.size() != 12) {
        return false;
    }

    for (std::size_t i = 0; i < 2; ++i) {
        if (!detail::encode_letter(isin[i]).has_value()) {
            return false;
        }
    }

    for (std::size_t i = 2; i < 11; ++i) {
        if (!detail::encode_alnum(isin[i]).has_value()) {
            return false;
        }
    }

    return detail::is_digit(isin[11]);
}

constexpr std::optional<int> calculate_check_digit(std::string_view isin_without_check_digit) noexcept {
    if (isin_without_check_digit.size() != 11) {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < 2; ++i) {
        if (!detail::encode_letter(isin_without_check_digit[i]).has_value()) {
            return std::nullopt;
        }
    }

    for (std::size_t i = 2; i < 11; ++i) {
        if (!detail::encode_alnum(isin_without_check_digit[i]).has_value()) {
            return std::nullopt;
        }
    }

    char expanded[22]{};
    std::size_t expanded_size = 0;
    for (char c : isin_without_check_digit) {
        const char upper = detail::to_upper_ascii(c);
        if (detail::is_digit(upper)) {
            expanded[expanded_size++] = upper;
            continue;
        }
        if (!detail::is_upper_alpha(upper)) {
            return std::nullopt;
        }

        const int value = 10 + (upper - 'A');
        expanded[expanded_size++] = static_cast<char>('0' + (value / 10));
        expanded[expanded_size++] = static_cast<char>('0' + (value % 10));
    }

    int sum = 0;
    bool double_digit = true;
    for (std::size_t i = expanded_size; i > 0; --i) {
        int digit = expanded[i - 1] - '0';
        if (double_digit) {
            digit *= 2;
        }
        sum += (digit / 10) + (digit % 10);
        double_digit = !double_digit;
    }

    return (10 - (sum % 10)) % 10;
}

constexpr bool has_valid_check_digit(std::string_view isin) noexcept {
    if (!is_valid_isin_format(isin)) {
        return false;
    }

    const auto expected = calculate_check_digit(isin.substr(0, 11));
    return expected.has_value() && *expected == (isin[11] - '0');
}

constexpr bool is_valid_isin(std::string_view isin) noexcept {
    return has_valid_check_digit(isin);
}

constexpr std::optional<value_type> encode_isin(std::string_view isin) noexcept {
    if (!is_valid_isin_format(isin)) {
        return std::nullopt;
    }

    value_type value = 0;

    value = value * 26 + *detail::encode_letter(isin[0]);
    value = value * 26 + *detail::encode_letter(isin[1]);

    for (std::size_t i = 2; i < 11; ++i) {
        value = value * 36 + *detail::encode_alnum(isin[i]);
    }

    value = value * 10 + static_cast<value_type>(isin[11] - '0');
    return value;
}

constexpr std::optional<value_type> encode_valid_isin(std::string_view isin) noexcept {
    if (!is_valid_isin(isin)) {
        return std::nullopt;
    }
    return encode_isin(isin);
}

constexpr std::optional<decoded_type> decode_isin(value_type value) noexcept {
    if (value > max_value) {
        return std::nullopt;
    }

    decoded_type out{};

    out[11] = static_cast<char>('0' + (value % 10));
    value /= 10;

    for (int i = 10; i >= 2; --i) {
        out[static_cast<std::size_t>(i)] = detail::decode_alnum(value % 36);
        value /= 36;
    }

    out[1] = detail::decode_letter(value % 26);
    value /= 26;
    out[0] = detail::decode_letter(value % 26);

    return out;
}

inline std::string to_string(const decoded_type& isin) {
    return std::string(isin.begin(), isin.end());
}

} // namespace secids::isin64

#endif
