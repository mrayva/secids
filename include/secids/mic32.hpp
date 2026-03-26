#ifndef SECIDS_MIC32_HPP
#define SECIDS_MIC32_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::mic32 {

using value_type = std::uint32_t;
using decoded_type = std::array<char, 4>;

namespace detail {

constexpr bool is_upper_alpha(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

constexpr char to_upper_ascii(char c) noexcept {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

constexpr bool is_mic_char(char c) noexcept {
    c = to_upper_ascii(c);
    return is_upper_alpha(c) || is_digit(c);
}

} // namespace detail

constexpr bool is_valid_mic_format(std::string_view mic) noexcept {
    if (mic.size() != 4) {
        return false;
    }
    for (char c : mic) {
        if (!detail::is_mic_char(c)) {
            return false;
        }
    }
    return true;
}

constexpr std::optional<value_type> pack_mic32(std::string_view mic) noexcept {
    if (!is_valid_mic_format(mic)) {
        return std::nullopt;
    }

    value_type value = 0;
    for (char c : mic) {
        value = static_cast<value_type>((value << 8U) | static_cast<unsigned char>(detail::to_upper_ascii(c)));
    }
    return value;
}

constexpr std::optional<decoded_type> unpack_mic32(value_type value) noexcept {
    decoded_type out{};
    for (int i = 3; i >= 0; --i) {
        out[static_cast<std::size_t>(i)] = static_cast<char>(value & 0xFFU);
        value >>= 8U;
    }

    for (char c : out) {
        if (!detail::is_mic_char(c)) {
            return std::nullopt;
        }
    }
    return out;
}

inline std::string to_string(const decoded_type& mic) {
    return std::string(mic.begin(), mic.end());
}

} // namespace secids::mic32

#endif
