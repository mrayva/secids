#ifndef SECIDS_FIGI64_HPP
#define SECIDS_FIGI64_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::figi64 {

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
constexpr value_type base21_1 = 21;
constexpr value_type base21_2 = pow_u64(base21_1, 2);
constexpr value_type base31_1 = 31;
constexpr value_type base31_8 = pow_u64(base31_1, 8);
constexpr value_type max_encoded_value = base21_2 * base31_8 * base10_1 - 1;

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

constexpr std::string_view consonants() noexcept {
    return "BCDFGHJKLMNPQRSTVWXYZ";
}

constexpr std::optional<value_type> encode_consonant(char c) noexcept {
    c = to_upper_ascii(c);
    const auto alphabet = consonants();
    for (value_type i = 0; i < alphabet.size(); ++i) {
        if (alphabet[i] == c) {
            return i;
        }
    }
    return std::nullopt;
}

constexpr std::optional<value_type> encode_body_char(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return static_cast<value_type>(c - '0');
    }
    const auto consonant = encode_consonant(c);
    if (!consonant.has_value()) {
        return std::nullopt;
    }
    return 10 + *consonant;
}

constexpr char decode_consonant(value_type v) noexcept {
    return consonants()[v];
}

constexpr char decode_body_char(value_type v) noexcept {
    if (v < 10) {
        return static_cast<char>('0' + v);
    }
    return decode_consonant(v - 10);
}

constexpr bool is_disallowed_prefix(char a, char b) noexcept {
    a = to_upper_ascii(a);
    b = to_upper_ascii(b);
    return (a == 'B' && b == 'S') ||
           (a == 'B' && b == 'M') ||
           (a == 'G' && b == 'G') ||
           (a == 'G' && b == 'B') ||
           (a == 'G' && b == 'H') ||
           (a == 'K' && b == 'Y') ||
           (a == 'V' && b == 'G');
}

constexpr std::optional<int> checksum_value(char c) noexcept {
    c = to_upper_ascii(c);
    if (is_digit(c)) {
        return c - '0';
    }
    if (is_upper_alpha(c) && !is_vowel(c)) {
        return 10 + (c - 'A');
    }
    return std::nullopt;
}

} // namespace detail

inline constexpr value_type max_value = detail::max_encoded_value;

constexpr bool is_valid_figi_format(std::string_view figi) noexcept {
    if (figi.size() != 12) {
        return false;
    }

    const auto prefix0 = detail::encode_consonant(figi[0]);
    const auto prefix1 = detail::encode_consonant(figi[1]);
    if (!prefix0.has_value() || !prefix1.has_value()) {
        return false;
    }
    if (detail::is_disallowed_prefix(figi[0], figi[1])) {
        return false;
    }

    if (detail::to_upper_ascii(figi[2]) != 'G') {
        return false;
    }

    for (std::size_t i = 3; i < 11; ++i) {
        if (!detail::encode_body_char(figi[i]).has_value()) {
            return false;
        }
    }

    return detail::is_digit(figi[11]);
}

constexpr std::optional<int> calculate_check_digit(std::string_view figi_without_check_digit) noexcept {
    if (figi_without_check_digit.size() != 11) {
        return std::nullopt;
    }
    if (!is_valid_figi_format(std::string(figi_without_check_digit).append("0"))) {
        return std::nullopt;
    }

    int sum = 0;
    bool double_digit = false;
    for (std::size_t i = figi_without_check_digit.size(); i > 0; --i) {
        const auto mapped = detail::checksum_value(figi_without_check_digit[i - 1]);
        if (!mapped.has_value()) {
            return std::nullopt;
        }
        int value = *mapped;
        if (double_digit) {
            value *= 2;
        }
        sum += (value / 10) + (value % 10);
        double_digit = !double_digit;
    }

    return (10 - (sum % 10)) % 10;
}

constexpr bool has_valid_check_digit(std::string_view figi) noexcept {
    if (!is_valid_figi_format(figi)) {
        return false;
    }

    const auto expected = calculate_check_digit(figi.substr(0, 11));
    return expected.has_value() && *expected == (figi[11] - '0');
}

constexpr bool is_valid_figi(std::string_view figi) noexcept {
    return has_valid_check_digit(figi);
}

constexpr std::optional<value_type> encode_figi(std::string_view figi) noexcept {
    if (!is_valid_figi_format(figi)) {
        return std::nullopt;
    }

    value_type value = 0;
    value = value * 21 + *detail::encode_consonant(figi[0]);
    value = value * 21 + *detail::encode_consonant(figi[1]);
    for (std::size_t i = 3; i < 11; ++i) {
        value = value * 31 + *detail::encode_body_char(figi[i]);
    }
    value = value * 10 + static_cast<value_type>(figi[11] - '0');
    return value;
}

constexpr std::optional<value_type> encode_valid_figi(std::string_view figi) noexcept {
    if (!is_valid_figi(figi)) {
        return std::nullopt;
    }
    return encode_figi(figi);
}

constexpr std::optional<decoded_type> decode_figi(value_type value) noexcept {
    if (value > max_value) {
        return std::nullopt;
    }

    decoded_type out{};
    out[11] = static_cast<char>('0' + (value % 10));
    value /= 10;

    for (int i = 10; i >= 3; --i) {
        out[static_cast<std::size_t>(i)] = detail::decode_body_char(value % 31);
        value /= 31;
    }

    out[2] = 'G';
    out[1] = detail::decode_consonant(value % 21);
    value /= 21;
    out[0] = detail::decode_consonant(value % 21);

    return out;
}

inline std::string to_string(const decoded_type& figi) {
    return std::string(figi.begin(), figi.end());
}

} // namespace secids::figi64

#endif
