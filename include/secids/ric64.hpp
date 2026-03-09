#ifndef SECIDS_RIC64_HPP
#define SECIDS_RIC64_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::ric64 {

using value_type = std::uint64_t;
using decoded_type = std::array<char, 8>;

enum class ric_kind : std::uint8_t {
    equity = 0,
    index = 1,
};

struct decoded_ric {
    decoded_type chars{};
    std::uint8_t length{};
    ric_kind kind{};
};

namespace detail {

constexpr value_type pow_u64(value_type base, unsigned exp) noexcept {
    value_type result = 1;
    for (unsigned i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

constexpr value_type base2_1 = 2;
constexpr value_type base4_1 = 4;
constexpr value_type base26_1 = 26;
constexpr value_type base26_2 = pow_u64(base26_1, 2);
constexpr value_type base26_4 = pow_u64(base26_1, 4);
constexpr value_type base36_1 = 36;
constexpr value_type base36_4 = pow_u64(base36_1, 4);
constexpr value_type equity_space = base4_1 * base2_1 * base26_4 * base26_2;
constexpr value_type index_space = base4_1 * base36_4;
constexpr value_type equity_offset = index_space;
constexpr value_type max_encoded_value = equity_offset + equity_space - 1;

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
    if (v < 10) {
        return static_cast<char>('0' + v);
    }
    return static_cast<char>('A' + (v - 10));
}

constexpr bool is_equity_format(std::string_view ric) noexcept {
    if (ric.size() < 3 || ric.size() > 7) {
        return false;
    }

    std::size_t dot_pos = std::string_view::npos;
    for (std::size_t i = 0; i < ric.size(); ++i) {
        if (ric[i] == '.') {
            if (dot_pos != std::string_view::npos) {
                return false;
            }
            dot_pos = i;
        }
    }

    if (dot_pos == std::string_view::npos) {
        return false;
    }

    const std::size_t root_len = dot_pos;
    const std::size_t suffix_len = ric.size() - dot_pos - 1;
    if (root_len < 1 || root_len > 4 || suffix_len < 1 || suffix_len > 2) {
        return false;
    }

    for (std::size_t i = 0; i < root_len; ++i) {
        if (!encode_letter(ric[i]).has_value()) {
            return false;
        }
    }
    for (std::size_t i = dot_pos + 1; i < ric.size(); ++i) {
        if (!encode_letter(ric[i]).has_value()) {
            return false;
        }
    }
    return true;
}

constexpr bool is_index_format(std::string_view ric) noexcept {
    if (ric.size() < 2 || ric.size() > 5 || ric[0] != '.') {
        return false;
    }
    for (std::size_t i = 1; i < ric.size(); ++i) {
        if (!encode_alnum(ric[i]).has_value()) {
            return false;
        }
    }
    return true;
}

} // namespace detail

inline constexpr value_type max_value = detail::max_encoded_value;

constexpr bool is_valid_ric_format(std::string_view ric) noexcept {
    return detail::is_equity_format(ric) || detail::is_index_format(ric);
}

constexpr bool is_equity_ric(std::string_view ric) noexcept {
    return detail::is_equity_format(ric);
}

constexpr bool is_index_ric(std::string_view ric) noexcept {
    return detail::is_index_format(ric);
}

constexpr std::optional<value_type> encode_ric(std::string_view ric) noexcept {
    if (detail::is_equity_format(ric)) {
        value_type value = 0;
        const auto dot_pos = ric.find('.');
        const auto root_len = dot_pos;
        const auto suffix_len = ric.size() - dot_pos - 1;

        value = 0;
        value = value * 4 + static_cast<value_type>(root_len - 1);
        value = value * 2 + static_cast<value_type>(suffix_len - 1);

        for (std::size_t i = 0; i < 4; ++i) {
            const char c = (i < root_len) ? ric[i] : 'A';
            value = value * 26 + *detail::encode_letter(c);
        }
        for (std::size_t i = 0; i < 2; ++i) {
            const char c = (i < suffix_len) ? ric[dot_pos + 1 + i] : 'A';
            value = value * 26 + *detail::encode_letter(c);
        }
        return detail::equity_offset + value;
    }

    if (detail::is_index_format(ric)) {
        value_type value = 0;
        const auto body_len = ric.size() - 1;

        value = 0;
        value = value * 4 + static_cast<value_type>(body_len - 1);
        for (std::size_t i = 0; i < 4; ++i) {
            const char c = (i < body_len) ? ric[1 + i] : '0';
            value = value * 36 + *detail::encode_alnum(c);
        }
        return value;
    }

    return std::nullopt;
}

constexpr std::optional<decoded_ric> decode_ric(value_type value) noexcept {
    if (value > max_value) {
        return std::nullopt;
    }

    decoded_ric out{};

    value_type tmp = value;
    value_type len_code = 0;
    value_type slots[6]{};

    if (value < detail::equity_offset) {
        tmp = value;
        for (int i = 3; i >= 0; --i) {
            slots[i] = tmp % 36;
            tmp /= 36;
        }
        len_code = tmp % 4;
        tmp /= 4;
        if (tmp != 0) {
            return std::nullopt;
        }
        out.kind = ric_kind::index;
        out.length = static_cast<std::uint8_t>(1 + (len_code + 1));
        out.chars[0] = '.';
        for (std::size_t i = 0; i < 4; ++i) {
            out.chars[1 + i] = detail::decode_alnum(slots[i]);
        }
        return out;
    }

    tmp = value - detail::equity_offset;
    for (int i = 5; i >= 0; --i) {
        slots[i] = tmp % 26;
        tmp /= 26;
    }
    value_type suffix_len_code = tmp % 2;
    tmp /= 2;
    value_type root_len_code = tmp % 4;
    tmp /= 4;
    if (tmp != 0) {
        return std::nullopt;
    }

    const std::size_t root_len = static_cast<std::size_t>(root_len_code + 1);
    const std::size_t suffix_len = static_cast<std::size_t>(suffix_len_code + 1);
    out.kind = ric_kind::equity;
    out.length = static_cast<std::uint8_t>(root_len + 1 + suffix_len);
    for (std::size_t i = 0; i < root_len; ++i) {
        out.chars[i] = detail::decode_letter(slots[i]);
    }
    out.chars[root_len] = '.';
    for (std::size_t i = 0; i < suffix_len; ++i) {
        out.chars[root_len + 1 + i] = detail::decode_letter(slots[4 + i]);
    }
    return out;
}

inline std::string to_string(const decoded_ric& ric) {
    return std::string(ric.chars.begin(), ric.chars.begin() + ric.length);
}

} // namespace secids::ric64

#endif
