#include <iostream>
#include <string_view>

#include "secids/cusip64.hpp"

namespace {

void print_usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " encode <CUSIP>\n"
        << "  " << argv0 << " encode-strict <CUSIP>\n"
        << "  " << argv0 << " decode <UINT64>\n"
        << "  " << argv0 << " check <CUSIP>\n"
        << "  " << argv0 << " check-digit <8-char-prefix>\n";
}

std::optional<secids::cusip64::value_type> parse_u64(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    secids::cusip64::value_type value = 0;
    for (char c : text) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        const auto digit = static_cast<secids::cusip64::value_type>(c - '0');
        const auto next = value * 10 + digit;
        if (next < value) {
            return std::nullopt;
        }
        value = next;
    }
    return value;
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
        const auto encoded = secids::cusip64::encode_cusip(argument);
        if (!encoded) {
            std::cerr << "invalid CUSIP format\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "encode-strict") {
        const auto encoded = secids::cusip64::encode_valid_cusip(argument);
        if (!encoded) {
            std::cerr << "invalid CUSIP or check digit\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "decode") {
        const auto value = parse_u64(argument);
        if (!value) {
            std::cerr << "invalid uint64 value\n";
            return 1;
        }
        const auto decoded = secids::cusip64::decode_cusip(*value);
        if (!decoded) {
            std::cerr << "encoded value out of range\n";
            return 1;
        }
        std::cout << secids::cusip64::to_string(*decoded) << '\n';
        return 0;
    }

    if (command == "check") {
        if (!secids::cusip64::is_valid_cusip_format(argument)) {
            std::cout << "format: invalid\n";
            std::cout << "check_digit: invalid\n";
            return 1;
        }

        std::cout << "format: valid\n";
        std::cout << "check_digit: "
                  << (secids::cusip64::has_valid_check_digit(argument) ? "valid" : "invalid")
                  << '\n';
        return secids::cusip64::has_valid_check_digit(argument) ? 0 : 1;
    }

    if (command == "check-digit") {
        const auto check_digit = secids::cusip64::calculate_check_digit(argument);
        if (!check_digit) {
            std::cerr << "invalid 8-character CUSIP prefix\n";
            return 1;
        }
        std::cout << *check_digit << '\n';
        return 0;
    }

    print_usage(argv[0]);
    return 2;
}
