#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "secids/cusip64.hpp"

namespace {

std::string make_valid_cusip(std::mt19937_64& rng) {
    static constexpr char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ*@#";
    std::uniform_int_distribution<int> dist(0, 38);

    std::string cusip(9, '0');
    for (std::size_t i = 0; i < 8; ++i) {
        cusip[i] = alphabet[dist(rng)];
    }

    const auto check_digit = secids::cusip64::calculate_check_digit(std::string_view{cusip}.substr(0, 8));
    assert(check_digit.has_value());
    cusip[8] = static_cast<char>('0' + *check_digit);
    return cusip;
}

} // namespace

int main() {
    using secids::cusip64::decode_cusip;
    using secids::cusip64::encode_cusip;
    using secids::cusip64::encode_valid_cusip;
    using secids::cusip64::has_valid_check_digit;
    using secids::cusip64::is_valid_cusip;
    using secids::cusip64::is_valid_cusip_format;
    using secids::cusip64::to_string;

    static_assert(secids::cusip64::max_value < UINT64_MAX);
    static_assert(is_valid_cusip("037833100"));
    static_assert(is_valid_cusip("17275R102"));
    static_assert(is_valid_cusip("38259P508"));
    static_assert(is_valid_cusip("594918104"));
    static_assert(is_valid_cusip("68389X105"));
    static_assert(is_valid_cusip("EJ7125481"));
    static_assert(!has_valid_check_digit("037833101"));
    static_assert(secids::cusip64::calculate_check_digit("03783310").value() == 0);
    static_assert(secids::cusip64::calculate_check_digit("68389X10").value() == 5);
    static_assert(secids::cusip64::calculate_check_digit("*@#ABCDE").has_value());
    static_assert(encode_valid_cusip("037833100").has_value());
    static_assert(!encode_valid_cusip("037833101").has_value());

    constexpr auto encoded = encode_cusip("037833100");
    static_assert(encoded.has_value());
    constexpr auto decoded = decode_cusip(*encoded);
    static_assert(decoded.has_value());
    static_assert((*decoded)[0] == '0');
    static_assert((*decoded)[8] == '0');

    for (std::string_view cusip : {
             std::string_view{"037833100"},
             std::string_view{"17275R102"},
             std::string_view{"38259P508"},
             std::string_view{"594918104"},
             std::string_view{"68389X105"},
             std::string_view{"EJ7125481"},
         }) {
        const auto value = encode_cusip(cusip);
        assert(value.has_value());
        const auto roundtrip = decode_cusip(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == cusip);
    }

    assert(is_valid_cusip_format("*@#ABCDE0"));
    assert(!encode_cusip("bad-cusip").has_value());
    assert(encode_cusip("037833101").has_value());
    assert(!encode_valid_cusip("037833101").has_value());
    assert(!is_valid_cusip("037833101"));
    assert(!decode_cusip(secids::cusip64::max_value + 1).has_value());

    std::mt19937_64 rng(0xC051F64ULL);
    for (int i = 0; i < 10000; ++i) {
        const auto cusip = make_valid_cusip(rng);
        assert(is_valid_cusip(cusip));

        const auto encoded_value = encode_cusip(cusip);
        assert(encoded_value.has_value());

        const auto decoded_value = decode_cusip(*encoded_value);
        assert(decoded_value.has_value());

        const auto roundtrip = to_string(*decoded_value);
        assert(roundtrip == cusip);
        assert(has_valid_check_digit(roundtrip));

        std::string invalid = cusip;
        invalid[8] = static_cast<char>('0' + ((invalid[8] - '0' + 1) % 10));
        assert(is_valid_cusip_format(invalid));
        assert(!has_valid_check_digit(invalid));
    }

    std::cout << "secids_cusip64_test passed\n";
    return 0;
}
