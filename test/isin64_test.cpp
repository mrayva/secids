#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "secids/isin64.hpp"

namespace {

std::string make_valid_isin(std::mt19937_64& rng) {
    static constexpr char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static constexpr char alnum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<int> letter_dist(0, 25);
    std::uniform_int_distribution<int> alnum_dist(0, 35);

    std::string isin(12, '0');
    isin[0] = letters[letter_dist(rng)];
    isin[1] = letters[letter_dist(rng)];
    for (std::size_t i = 2; i < 11; ++i) {
        isin[i] = alnum[alnum_dist(rng)];
    }

    const auto check_digit = secids::isin64::calculate_check_digit(std::string_view{isin}.substr(0, 11));
    assert(check_digit.has_value());
    isin[11] = static_cast<char>('0' + *check_digit);
    return isin;
}

} // namespace

int main() {
    using secids::isin64::decode_isin;
    using secids::isin64::encode_isin;
    using secids::isin64::encode_valid_isin;
    using secids::isin64::has_valid_check_digit;
    using secids::isin64::is_valid_isin;
    using secids::isin64::is_valid_isin_format;
    using secids::isin64::to_string;

    static_assert(secids::isin64::max_value < UINT64_MAX);
    static_assert(is_valid_isin("US0378331005"));
    static_assert(is_valid_isin_format("US0378331005"));
    static_assert(!is_valid_isin_format("U70378331005"));
    static_assert(!has_valid_check_digit("US0378331004"));
    static_assert(secids::isin64::calculate_check_digit("US037833100").value() == 5);
    static_assert(encode_valid_isin("US0378331005").has_value());
    static_assert(!encode_valid_isin("US0378331004").has_value());

    constexpr auto encoded = encode_isin("US0378331005");
    static_assert(encoded.has_value());
    constexpr auto decoded = decode_isin(*encoded);
    static_assert(decoded.has_value());
    static_assert((*decoded)[0] == 'U');
    static_assert((*decoded)[11] == '5');

    for (std::string_view isin : {
             std::string_view{"US0378331005"},
             std::string_view{"US5949181045"},
             std::string_view{"GB0002634946"},
             std::string_view{"AU0000XVGZA3"},
             std::string_view{"XS0971721963"},
         }) {
        const auto value = encode_isin(isin);
        assert(value.has_value());
        const auto roundtrip = decode_isin(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == isin);
    }

    assert(!encode_isin("us0378331005!").has_value());
    assert(encode_isin("US0378331004").has_value());
    assert(!encode_valid_isin("US0378331004").has_value());
    assert(!is_valid_isin("US0378331004"));
    assert(!decode_isin(secids::isin64::max_value + 1).has_value());

    std::mt19937_64 rng(0x5EC1D5ULL);
    for (int i = 0; i < 10000; ++i) {
        const auto isin = make_valid_isin(rng);
        assert(is_valid_isin(isin));

        const auto encoded_value = encode_isin(isin);
        assert(encoded_value.has_value());

        const auto decoded_value = decode_isin(*encoded_value);
        assert(decoded_value.has_value());

        const auto roundtrip = to_string(*decoded_value);
        assert(roundtrip == isin);
        assert(has_valid_check_digit(roundtrip));

        std::string invalid = isin;
        invalid[11] = static_cast<char>('0' + ((invalid[11] - '0' + 1) % 10));
        assert(is_valid_isin_format(invalid));
        assert(!has_valid_check_digit(invalid));
    }

    std::cout << "secids_isin64_test passed\n";
    return 0;
}
