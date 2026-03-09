#include <iostream>
#include <string_view>

#include "secids/detail/cli_support.hpp"
#include "secids/isin64.hpp"

namespace {

void print_usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " encode <ISIN>\n"
        << "  " << argv0 << " encode-strict <ISIN>\n"
        << "  " << argv0 << " decode <UINT64>\n"
        << "  " << argv0 << " check <ISIN>\n"
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
        const auto encoded = secids::isin64::encode_isin(argument);
        if (!encoded) {
            std::cerr << "invalid ISIN format\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "encode-strict") {
        const auto encoded = secids::isin64::encode_valid_isin(argument);
        if (!encoded) {
            std::cerr << "invalid ISIN or check digit\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "decode") {
        const auto value = secids::detail::parse_u64<secids::isin64::value_type>(argument);
        if (!value) {
            std::cerr << "invalid uint64 value\n";
            return 1;
        }
        const auto decoded = secids::isin64::decode_isin(*value);
        if (!decoded) {
            std::cerr << "encoded value out of range\n";
            return 1;
        }
        std::cout << secids::isin64::to_string(*decoded) << '\n';
        return 0;
    }

    if (command == "check") {
        if (!secids::isin64::is_valid_isin_format(argument)) {
            std::cout << "format: invalid\n";
            std::cout << "check_digit: invalid\n";
            return 1;
        }

        std::cout << "format: valid\n";
        std::cout << "check_digit: "
                  << (secids::isin64::has_valid_check_digit(argument) ? "valid" : "invalid")
                  << '\n';
        return secids::isin64::has_valid_check_digit(argument) ? 0 : 1;
    }

    if (command == "check-digit") {
        const auto check_digit = secids::isin64::calculate_check_digit(argument);
        if (!check_digit) {
            std::cerr << "invalid 11-character ISIN prefix\n";
            return 1;
        }
        std::cout << *check_digit << '\n';
        return 0;
    }

    print_usage(argv[0]);
    return 2;
}
