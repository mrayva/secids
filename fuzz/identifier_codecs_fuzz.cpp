#include <cstddef>
#include <cstdint>
#include <string_view>

#include "secids/secids.hpp"

namespace {

std::uint64_t read_u64(const std::uint8_t* data, std::size_t size) {
    std::uint64_t value = 0;
    const std::size_t count = size < 8 ? size : 8;
    for (std::size_t i = 0; i < count; ++i) {
        value |= static_cast<std::uint64_t>(data[i]) << (i * 8U);
    }
    return value;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view input(reinterpret_cast<const char*>(data), size);
    const std::uint64_t value = read_u64(data, size);

    static_cast<void>(secids::isin64::encode_isin(input));
    static_cast<void>(secids::cusip64::encode_cusip(input));
    static_cast<void>(secids::sedol64::encode_sedol(input));
    static_cast<void>(secids::figi64::encode_figi(input));
    static_cast<void>(secids::ric64::encode_ric(input));
    static_cast<void>(secids::mic32::pack_mic32(input));

    if (const auto decoded = secids::isin64::decode_isin(value)) {
        const auto encoded = secids::isin64::encode_isin(secids::isin64::to_string(*decoded));
        if (!encoded || *encoded != value) __builtin_trap();
    }
    if (const auto decoded = secids::cusip64::decode_cusip(value)) {
        const auto encoded = secids::cusip64::encode_cusip(secids::cusip64::to_string(*decoded));
        if (!encoded || *encoded != value) __builtin_trap();
    }
    if (const auto decoded = secids::sedol64::decode_sedol(value)) {
        const auto encoded = secids::sedol64::encode_sedol(secids::sedol64::to_string(*decoded));
        if (!encoded || *encoded != value) __builtin_trap();
    }
    if (const auto decoded = secids::figi64::decode_figi(value)) {
        const auto encoded = secids::figi64::encode_figi(secids::figi64::to_string(*decoded));
        if (!encoded || *encoded != value) __builtin_trap();
    }
    if (const auto decoded = secids::ric64::decode_ric(value)) {
        const auto encoded = secids::ric64::encode_ric(secids::ric64::to_string(*decoded));
        if (!encoded || *encoded != value) __builtin_trap();
    }

    const auto mic_value = static_cast<std::uint32_t>(value);
    if (const auto decoded = secids::mic32::unpack_mic32(mic_value)) {
        const auto encoded = secids::mic32::pack_mic32(secids::mic32::to_string(*decoded));
        if (!encoded || *encoded != mic_value) __builtin_trap();
    }
    return 0;
}
