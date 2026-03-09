#include <iostream>
#include <string_view>

#include "secids/detail/cli_support.hpp"
#include "secids/figi64.hpp"

namespace {

void print_usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " encode <FIGI>\n"
        << "  " << argv0 << " encode-strict <FIGI>\n"
        << "  " << argv0 << " decode <UINT64>\n"
        << "  " << argv0 << " check <FIGI>\n"
        << "  " << argv0 << " check-digit <11-char-prefix>\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string_view command = argv[1];
    const std::string_view argument = argv[2];

    if (command == "encode") {
        const auto encoded = secids::figi64::encode_figi(argument);
        if (!encoded) {
            std::cerr << "invalid FIGI format\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "encode-strict") {
        const auto encoded = secids::figi64::encode_valid_figi(argument);
        if (!encoded) {
            std::cerr << "invalid FIGI or check digit\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "decode") {
        const auto value = secids::detail::parse_u64<secids::figi64::value_type>(argument);
        if (!value) {
            std::cerr << "invalid uint64 value\n";
            return 1;
        }
        const auto decoded = secids::figi64::decode_figi(*value);
        if (!decoded) {
            std::cerr << "encoded value out of range\n";
            return 1;
        }
        std::cout << secids::figi64::to_string(*decoded) << '\n';
        return 0;
    }

    if (command == "check") {
        if (!secids::figi64::is_valid_figi_format(argument)) {
            std::cout << "format: invalid\n";
            std::cout << "check_digit: invalid\n";
            return 1;
        }

        std::cout << "format: valid\n";
        std::cout << "check_digit: "
                  << (secids::figi64::has_valid_check_digit(argument) ? "valid" : "invalid")
                  << '\n';
        return secids::figi64::has_valid_check_digit(argument) ? 0 : 1;
    }

    if (command == "check-digit") {
        const auto check_digit = secids::figi64::calculate_check_digit(argument);
        if (!check_digit) {
            std::cerr << "invalid 11-character FIGI prefix\n";
            return 1;
        }
        std::cout << *check_digit << '\n';
        return 0;
    }

    print_usage(argv[0]);
    return 2;
}
