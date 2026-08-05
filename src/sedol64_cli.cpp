#include <optional>
#include <string>
#include <string_view>

#include "secids/detail/cli_support.hpp"
#include "secids/sedol64.hpp"

namespace {

struct sedol_cli_traits {
    static constexpr const char* name = "SEDOL";
    static constexpr int prefix_len = 6;
    using value_type = secids::sedol64::value_type;

    static std::optional<value_type> encode(std::string_view s) { return secids::sedol64::encode_sedol(s); }
    static std::optional<value_type> encode_valid(std::string_view s) { return secids::sedol64::encode_valid_sedol(s); }
    static std::optional<secids::sedol64::decoded_type> decode(value_type v) { return secids::sedol64::decode_sedol(v); }
    static bool is_valid_format(std::string_view s) { return secids::sedol64::is_valid_sedol_format(s); }
    static bool has_valid_check_digit(std::string_view s) { return secids::sedol64::has_valid_check_digit(s); }
    static std::optional<int> calculate_check_digit(std::string_view s) { return secids::sedol64::calculate_check_digit(s); }
    static std::string to_string(const secids::sedol64::decoded_type& d) { return secids::sedol64::to_string(d); }
};

} // namespace

int main(int argc, char** argv) {
    return secids::detail::run_identifier_cli<sedol_cli_traits>(argc, argv);
}
