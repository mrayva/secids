#include <optional>
#include <string>
#include <string_view>

#include "secids/detail/cli_support.hpp"
#include "secids/isin64.hpp"

namespace {

struct isin_cli_traits {
    static constexpr const char* name = "ISIN";
    static constexpr int prefix_len = 11;
    using value_type = secids::isin64::value_type;

    static std::optional<value_type> encode(std::string_view s) { return secids::isin64::encode_isin(s); }
    static std::optional<value_type> encode_valid(std::string_view s) { return secids::isin64::encode_valid_isin(s); }
    static std::optional<secids::isin64::decoded_type> decode(value_type v) { return secids::isin64::decode_isin(v); }
    static bool is_valid_format(std::string_view s) { return secids::isin64::is_valid_isin_format(s); }
    static bool has_valid_check_digit(std::string_view s) { return secids::isin64::has_valid_check_digit(s); }
    static std::optional<int> calculate_check_digit(std::string_view s) { return secids::isin64::calculate_check_digit(s); }
    static std::string to_string(const secids::isin64::decoded_type& d) { return secids::isin64::to_string(d); }
};

} // namespace

int main(int argc, char** argv) {
    return secids::detail::run_identifier_cli<isin_cli_traits>(argc, argv);
}
