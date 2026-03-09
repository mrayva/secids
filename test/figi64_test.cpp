#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "secids/figi64.hpp"

namespace {

std::string make_valid_figi(std::mt19937_64& rng) {
    static constexpr char consonants[] = "BCDFGHJKLMNPQRSTVWXYZ";
    static constexpr char body[] = "0123456789BCDFGHJKLMNPQRSTVWXYZ";
    std::uniform_int_distribution<int> prefix_dist(0, 20);
    std::uniform_int_distribution<int> body_dist(0, 30);

    std::string figi(12, '0');
    do {
        figi[0] = consonants[prefix_dist(rng)];
        figi[1] = consonants[prefix_dist(rng)];
    } while (!secids::figi64::is_valid_figi_format(std::string(figi, 0, 2) + "G000000000"));
    figi[2] = 'G';
    for (std::size_t i = 3; i < 11; ++i) {
        figi[i] = body[body_dist(rng)];
    }

    const auto check_digit = secids::figi64::calculate_check_digit(std::string_view{figi}.substr(0, 11));
    assert(check_digit.has_value());
    figi[11] = static_cast<char>('0' + *check_digit);
    return figi;
}

} // namespace

int main() {
    using secids::figi64::decode_figi;
    using secids::figi64::encode_figi;
    using secids::figi64::encode_valid_figi;
    using secids::figi64::has_valid_check_digit;
    using secids::figi64::is_valid_figi;
    using secids::figi64::is_valid_figi_format;
    using secids::figi64::to_string;

    static_assert(secids::figi64::max_value < UINT64_MAX);
    static_assert(is_valid_figi("BBG000BLNNV0"));
    static_assert(is_valid_figi("BBG000BLNQ16"));
    static_assert(is_valid_figi("BBG000BLNR78"));
    static_assert(is_valid_figi("BBG000BLNPB7"));
    static_assert(is_valid_figi("BBG000BLNWJ4"));
    static_assert(is_valid_figi("BBG000BLNXP5"));
    static_assert(!is_valid_figi_format("BAG000BLNNV0"));
    static_assert(!is_valid_figi_format("BSG000BLNNV0"));
    static_assert(!has_valid_check_digit("BBG000BLNNV1"));
    static_assert(secids::figi64::calculate_check_digit("BBG000BLNNV").value() == 0);
    static_assert(encode_valid_figi("BBG000BLNNV0").has_value());
    static_assert(!encode_valid_figi("BBG000BLNNV1").has_value());

    constexpr auto encoded = encode_figi("BBG000BLNNV0");
    static_assert(encoded.has_value());
    constexpr auto decoded = decode_figi(*encoded);
    static_assert(decoded.has_value());
    static_assert((*decoded)[0] == 'B');
    static_assert((*decoded)[2] == 'G');
    static_assert((*decoded)[11] == '0');

    for (std::string_view figi : {
             std::string_view{"BBG000BLNNV0"},
             std::string_view{"BBG000BLNQ16"},
             std::string_view{"BBG000BLNR78"},
             std::string_view{"BBG000BLNPB7"},
         }) {
        const auto value = encode_figi(figi);
        assert(value.has_value());
        const auto roundtrip = decode_figi(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == figi);
    }

    assert(!encode_figi("BAG000BLNNV0").has_value());
    assert(encode_figi("BBG000BLNNV1").has_value());
    assert(!encode_valid_figi("BBG000BLNNV1").has_value());
    assert(!is_valid_figi("BBG000BLNNV1"));
    assert(!decode_figi(secids::figi64::max_value + 1).has_value());

    std::mt19937_64 rng(0xF16164ULL);
    for (int i = 0; i < 10000; ++i) {
        const auto figi = make_valid_figi(rng);
        assert(is_valid_figi(figi));

        const auto encoded_value = encode_figi(figi);
        assert(encoded_value.has_value());

        const auto decoded_value = decode_figi(*encoded_value);
        assert(decoded_value.has_value());

        const auto roundtrip = to_string(*decoded_value);
        assert(roundtrip == figi);
        assert(has_valid_check_digit(roundtrip));

        std::string invalid = figi;
        invalid[11] = static_cast<char>('0' + ((invalid[11] - '0' + 1) % 10));
        assert(is_valid_figi_format(invalid));
        assert(!has_valid_check_digit(invalid));
    }

    std::cout << "secids_figi64_test passed\n";
    return 0;
}
