#include <optional>
#include <string>
#include <string_view>

#include "secids/detail/cli_support.hpp"
#include "secids/figi64.hpp"

namespace {

struct figi_cli_traits {
    static constexpr const char* name = "FIGI";
    static constexpr int prefix_len = 11;
    using value_type = secids::figi64::value_type;

    static std::optional<value_type> encode(std::string_view s) { return secids::figi64::encode_figi(s); }
    static std::optional<value_type> encode_valid(std::string_view s) { return secids::figi64::encode_valid_figi(s); }
    static std::optional<secids::figi64::decoded_type> decode(value_type v) { return secids::figi64::decode_figi(v); }
    static bool is_valid_format(std::string_view s) { return secids::figi64::is_valid_figi_format(s); }
    static bool has_valid_check_digit(std::string_view s) { return secids::figi64::has_valid_check_digit(s); }
    static std::optional<int> calculate_check_digit(std::string_view s) { return secids::figi64::calculate_check_digit(s); }
    static std::string to_string(const secids::figi64::decoded_type& d) { return secids::figi64::to_string(d); }
};

} // namespace

int main(int argc, char** argv) {
    return secids::detail::run_identifier_cli<figi_cli_traits>(argc, argv);
}
