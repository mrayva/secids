#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "secids/ric64.hpp"

namespace {

std::string make_equity_ric(std::mt19937_64& rng) {
    static constexpr char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<int> root_len_dist(1, 4);
    std::uniform_int_distribution<int> suffix_len_dist(1, 2);
    std::uniform_int_distribution<int> letter_dist(0, 25);

    const int root_len = root_len_dist(rng);
    const int suffix_len = suffix_len_dist(rng);
    std::string out;
    out.reserve(7);
    for (int i = 0; i < root_len; ++i) {
        out.push_back(letters[letter_dist(rng)]);
    }
    out.push_back('.');
    for (int i = 0; i < suffix_len; ++i) {
        out.push_back(letters[letter_dist(rng)]);
    }
    return out;
}

std::string make_index_ric(std::mt19937_64& rng) {
    static constexpr char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<int> len_dist(1, 4);
    std::uniform_int_distribution<int> char_dist(0, 35);

    const int len = len_dist(rng);
    std::string out;
    out.reserve(5);
    out.push_back('.');
    for (int i = 0; i < len; ++i) {
        out.push_back(alphabet[char_dist(rng)]);
    }
    return out;
}

} // namespace

int main() {
    using secids::ric64::decode_ric;
    using secids::ric64::encode_ric;
    using secids::ric64::is_equity_ric;
    using secids::ric64::is_index_ric;
    using secids::ric64::is_valid_ric_format;
    using secids::ric64::to_string;

    static_assert(secids::ric64::max_value < UINT64_MAX);
    static_assert(is_valid_ric_format("IBM.N"));
    static_assert(is_valid_ric_format("WMT.N"));
    static_assert(is_valid_ric_format("VOD.L"));
    static_assert(is_valid_ric_format("AAPL.OQ"));
    static_assert(is_valid_ric_format(".DJI"));
    static_assert(is_valid_ric_format(".SPX"));
    static_assert(is_valid_ric_format(".FTSE"));
    static_assert(is_equity_ric("IBM.N"));
    static_assert(is_index_ric(".DJI"));
    static_assert(!is_valid_ric_format("IBM"));
    static_assert(!is_valid_ric_format("IBM.NYQ"));
    static_assert(!is_valid_ric_format("IBM1.N"));
    static_assert(!is_valid_ric_format("..DJI"));

    constexpr auto equity_encoded = encode_ric("IBM.N");
    static_assert(equity_encoded.has_value());
    constexpr auto equity_decoded = decode_ric(*equity_encoded);
    static_assert(equity_decoded.has_value());

    constexpr auto index_encoded = encode_ric(".DJI");
    static_assert(index_encoded.has_value());
    constexpr auto index_decoded = decode_ric(*index_encoded);
    static_assert(index_decoded.has_value());

    for (std::string_view ric : {
             std::string_view{"IBM.N"},
             std::string_view{"WMT.N"},
             std::string_view{"AAPL.OQ"},
             std::string_view{".DJI"},
             std::string_view{".SPX"},
             std::string_view{".NDX"},
         }) {
        const auto value = encode_ric(ric);
        assert(value.has_value());
        const auto roundtrip = decode_ric(*value);
        assert(roundtrip.has_value());
        assert(to_string(*roundtrip) == ric);
    }

    std::mt19937_64 rng(0xA1C64ULL);
    for (int i = 0; i < 10000; ++i) {
        const auto equity = make_equity_ric(rng);
        const auto equity_value = encode_ric(equity);
        assert(equity_value.has_value());
        const auto equity_roundtrip = decode_ric(*equity_value);
        assert(equity_roundtrip.has_value());
        assert(to_string(*equity_roundtrip) == equity);

        const auto index = make_index_ric(rng);
        const auto index_value = encode_ric(index);
        assert(index_value.has_value());
        const auto index_roundtrip = decode_ric(*index_value);
        assert(index_roundtrip.has_value());
        assert(to_string(*index_roundtrip) == index);
    }

    std::cout << "secids_ric64_test passed\n";
    return 0;
}
