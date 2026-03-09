#ifndef SECIDS_DETAIL_CLI_SUPPORT_HPP
#define SECIDS_DETAIL_CLI_SUPPORT_HPP

#include <cstdint>
#include <optional>
#include <string_view>

namespace secids::detail {

template <typename UInt>
std::optional<UInt> parse_u64(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    UInt value = 0;
    for (char c : text) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        const auto digit = static_cast<UInt>(c - '0');
        const auto next = value * static_cast<UInt>(10) + digit;
        if (next < value) {
            return std::nullopt;
        }
        value = next;
    }
    return value;
}

} // namespace secids::detail

#endif
