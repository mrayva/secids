#include <optional>
#include <string>
#include <string_view>

#include "secids/detail/cli_support.hpp"
#include "secids/cusip64.hpp"

namespace {

struct cusip_cli_traits {
    static constexpr const char* name = "CUSIP";
    static constexpr int prefix_len = 8;
    using value_type = secids::cusip64::value_type;

    static std::optional<value_type> encode(std::string_view s) { return secids::cusip64::encode_cusip(s); }
    static std::optional<value_type> encode_valid(std::string_view s) { return secids::cusip64::encode_valid_cusip(s); }
    static std::optional<secids::cusip64::decoded_type> decode(value_type v) { return secids::cusip64::decode_cusip(v); }
    static bool is_valid_format(std::string_view s) { return secids::cusip64::is_valid_cusip_format(s); }
    static bool has_valid_check_digit(std::string_view s) { return secids::cusip64::has_valid_check_digit(s); }
    static std::optional<int> calculate_check_digit(std::string_view s) { return secids::cusip64::calculate_check_digit(s); }
    static std::string to_string(const secids::cusip64::decoded_type& d) { return secids::cusip64::to_string(d); }
};

} // namespace

int main(int argc, char** argv) {
    return secids::detail::run_identifier_cli<cusip_cli_traits>(argc, argv);
}
