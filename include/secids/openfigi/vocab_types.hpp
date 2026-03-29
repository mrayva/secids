#ifndef SECIDS_OPENFIGI_VOCAB_TYPES_HPP
#define SECIDS_OPENFIGI_VOCAB_TYPES_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace secids::openfigi {

enum class vocab_domain : std::uint16_t {
    mic_code = 0,
    exch_code = 1,
    security_type = 2,
    market_sec_des = 3,
    security_type2 = 4,
};

struct vocab_id {
    vocab_domain domain;
    std::uint32_t value;
};

inline constexpr std::array<vocab_domain, 5> all_vocab_domains{
    vocab_domain::mic_code,
    vocab_domain::exch_code,
    vocab_domain::security_type,
    vocab_domain::market_sec_des,
    vocab_domain::security_type2,
};

constexpr std::string_view to_string(vocab_domain domain) noexcept {
    switch (domain) {
        case vocab_domain::mic_code:
            return "micCode";
        case vocab_domain::exch_code:
            return "exchCode";
        case vocab_domain::security_type:
            return "securityType";
        case vocab_domain::market_sec_des:
            return "marketSecDes";
        case vocab_domain::security_type2:
            return "securityType2";
    }
    return "unknown";
}

constexpr std::optional<vocab_domain> parse_vocab_domain(std::string_view value) noexcept {
    for (const auto domain : all_vocab_domains) {
        if (to_string(domain) == value) {
            return domain;
        }
    }
    return std::nullopt;
}

} // namespace secids::openfigi

#endif
