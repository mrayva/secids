#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "secids/mic32.hpp"

namespace {

std::string make_valid_mic(std::mt19937_64& rng) {
    static constexpr char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<int> dist(0, 35);

    std::string mic(4, '0');
    for (char& c : mic) {
        c = alphabet[dist(rng)];
    }
    return mic;
}

} // namespace

int main() {
    using secids::mic32::pack_mic32;
    using secids::mic32::to_string;
    using secids::mic32::unpack_mic32;

    static_assert(secids::mic32::is_valid_mic_format("XNAS"));
    static_assert(secids::mic32::is_valid_mic_format("XNYS"));
    static_assert(secids::mic32::is_valid_mic_format("24EQ"));
    static_assert(!secids::mic32::is_valid_mic_format("XN S"));
    static_assert(!secids::mic32::is_valid_mic_format("XNASX"));
    static_assert(pack_mic32("XNAS").has_value());
    static_assert(pack_mic32("xnas").has_value());
    static_assert(!pack_mic32("XN S").has_value());

    constexpr auto packed = pack_mic32("XNAS");
    static_assert(packed.has_value());
    constexpr auto unpacked = unpack_mic32(*packed);
    static_assert(unpacked.has_value());
    static_assert((*unpacked)[0] == 'X');
    static_assert((*unpacked)[3] == 'S');

    for (std::string_view mic : {
             std::string_view{"XNAS"},
             std::string_view{"XNYS"},
             std::string_view{"XLON"},
             std::string_view{"XPAR"},
             std::string_view{"24EQ"},
         }) {
        const auto value = pack_mic32(mic);
        assert(value.has_value());
        const auto roundtrip = unpack_mic32(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == std::string(mic));
    }

    assert(!unpack_mic32(0U).has_value());

    std::mt19937_64 rng(0xB16C320ULL);
    for (int i = 0; i < 10000; ++i) {
        const auto mic = make_valid_mic(rng);
        const auto value = pack_mic32(mic);
        assert(value.has_value());

        const auto roundtrip = unpack_mic32(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == mic);
    }

    std::cout << "secids_mic32_test passed\n";
    return 0;
}
