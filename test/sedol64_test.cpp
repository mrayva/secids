#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "secids/sedol64.hpp"

namespace {

std::string make_valid_sedol(std::mt19937_64& rng) {
    static constexpr char alphabet[] = "0123456789BCDFGHJKLMNPQRSTVWXYZ";
    std::uniform_int_distribution<int> dist(0, 30);

    std::string sedol(7, '0');
    for (std::size_t i = 0; i < 6; ++i) {
        sedol[i] = alphabet[dist(rng)];
    }

    const auto check_digit = secids::sedol64::calculate_check_digit(std::string_view{sedol}.substr(0, 6));
    assert(check_digit.has_value());
    sedol[6] = static_cast<char>('0' + *check_digit);
    return sedol;
}

} // namespace

int main() {
    using secids::sedol64::decode_sedol;
    using secids::sedol64::encode_sedol;
    using secids::sedol64::encode_valid_sedol;
    using secids::sedol64::has_valid_check_digit;
    using secids::sedol64::is_valid_sedol;
    using secids::sedol64::is_valid_sedol_format;
    using secids::sedol64::to_string;

    static_assert(secids::sedol64::max_value < UINT64_MAX);
    static_assert(is_valid_sedol("0263494"));
    static_assert(is_valid_sedol("2936921"));
    static_assert(is_valid_sedol("B0YBKJ7"));
    static_assert(!has_valid_check_digit("1234567"));
    static_assert(!is_valid_sedol_format("B0YBEU9"));
    static_assert(secids::sedol64::calculate_check_digit("026349").value() == 4);
    static_assert(secids::sedol64::calculate_check_digit("B0YBKJ").value() == 7);
    static_assert(encode_valid_sedol("0263494").has_value());
    static_assert(!encode_valid_sedol("0263495").has_value());

    constexpr auto encoded = encode_sedol("0263494");
    static_assert(encoded.has_value());
    constexpr auto decoded = decode_sedol(*encoded);
    static_assert(decoded.has_value());
    static_assert((*decoded)[0] == '0');
    static_assert((*decoded)[6] == '4');

    for (std::string_view sedol : {
             std::string_view{"0263494"},
             std::string_view{"2936921"},
             std::string_view{"B0YBKJ7"},
         }) {
        const auto value = encode_sedol(sedol);
        assert(value.has_value());
        const auto roundtrip = decode_sedol(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == sedol);
    }

    assert(!encode_sedol("ABCDEF1").has_value());
    assert(encode_sedol("b0ybkj7").has_value());
    assert(encode_sedol("2936922").has_value());
    assert(!encode_valid_sedol("2936922").has_value());
    assert(!is_valid_sedol("2936922"));
    assert(!decode_sedol(secids::sedol64::max_value + 1).has_value());

    std::mt19937_64 rng(0x5ED014ULL);
    for (int i = 0; i < 10000; ++i) {
        const auto sedol = make_valid_sedol(rng);
        assert(is_valid_sedol(sedol));

        const auto encoded_value = encode_sedol(sedol);
        assert(encoded_value.has_value());

        const auto decoded_value = decode_sedol(*encoded_value);
        assert(decoded_value.has_value());

        const auto roundtrip = to_string(*decoded_value);
        assert(roundtrip == sedol);
        assert(has_valid_check_digit(roundtrip));

        std::string invalid = sedol;
        invalid[6] = static_cast<char>('0' + ((invalid[6] - '0' + 1) % 10));
        assert(is_valid_sedol_format(invalid));
        assert(!has_valid_check_digit(invalid));
    }

    std::uniform_int_distribution<secids::sedol64::value_type> any_value_dist(0, secids::sedol64::max_value);
    for (int i = 0; i < 10000; ++i) {
        const auto value = any_value_dist(rng);
        const auto decoded_value = decode_sedol(value);
        assert(decoded_value.has_value());
        const auto reencoded_value = encode_sedol(to_string(*decoded_value));
        assert(reencoded_value.has_value());
        assert(*reencoded_value == value);
    }

    std::cout << "secids_sedol64_test passed\n";
    return 0;
}
