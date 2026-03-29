#ifndef SECIDS_DETAIL_ALPHA_CODE16_HPP
#define SECIDS_DETAIL_ALPHA_CODE16_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace secids::detail::alpha_code16 {

constexpr bool is_upper_alpha(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

constexpr char to_upper_ascii(char c) noexcept {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

template <std::size_t Length>
constexpr std::optional<std::uint16_t> pack(std::string_view code) noexcept {
    static_assert(Length > 0);
    if (code.size() != Length) {
        return std::nullopt;
    }

    std::uint16_t value = 0;
    for (char c : code) {
        c = to_upper_ascii(c);
        if (!is_upper_alpha(c)) {
            return std::nullopt;
        }
        value = static_cast<std::uint16_t>(value * 26U + static_cast<std::uint16_t>(c - 'A'));
    }
    return value;
}

template <std::size_t Length>
inline std::string unpack(std::uint16_t value) {
    static_assert(Length > 0);
    std::string out(Length, '\0');
    for (std::size_t i = Length; i > 0; --i) {
        out[i - 1] = static_cast<char>('A' + (value % 26U));
        value = static_cast<std::uint16_t>(value / 26U);
    }
    return out;
}

} // namespace secids::detail::alpha_code16

#endif
