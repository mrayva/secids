#ifndef SECIDS_DETAIL_CLI_SUPPORT_HPP
#define SECIDS_DETAIL_CLI_SUPPORT_HPP

#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string_view>

namespace secids::detail {

template <typename UInt>
std::optional<UInt> parse_u64(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    constexpr UInt max_value = std::numeric_limits<UInt>::max();

    UInt value = 0;
    for (char c : text) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        const auto digit = static_cast<UInt>(c - '0');
        if (value > (max_value - digit) / static_cast<UInt>(10)) {
            return std::nullopt;
        }
        value = value * static_cast<UInt>(10) + digit;
    }
    return value;
}

// Shared driver for the encode / encode-strict / decode / check / check-digit
// CLIs that ISIN, CUSIP, SEDOL, and FIGI all expose with identical shape.
// `Traits` supplies the identifier-specific names and codec functions; see
// e.g. isin64_cli.cpp for an instantiation.
template <typename Traits>
void print_identifier_cli_usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " encode <" << Traits::name << ">\n"
        << "  " << argv0 << " encode-strict <" << Traits::name << ">\n"
        << "  " << argv0 << " decode <UINT64>\n"
        << "  " << argv0 << " check <" << Traits::name << ">\n"
        << "  " << argv0 << " check-digit <" << Traits::prefix_len << "-char-prefix>\n";
}

template <typename Traits>
int run_identifier_cli(int argc, char** argv) {
    if (argc != 3) {
        print_identifier_cli_usage<Traits>(argv[0]);
        return 2;
    }

    const std::string_view command = argv[1];
    const std::string_view argument = argv[2];

    if (command == "encode") {
        const auto encoded = Traits::encode(argument);
        if (!encoded) {
            std::cerr << "invalid " << Traits::name << " format\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "encode-strict") {
        const auto encoded = Traits::encode_valid(argument);
        if (!encoded) {
            std::cerr << "invalid " << Traits::name << " or check digit\n";
            return 1;
        }
        std::cout << *encoded << '\n';
        return 0;
    }

    if (command == "decode") {
        const auto value = parse_u64<typename Traits::value_type>(argument);
        if (!value) {
            std::cerr << "invalid uint64 value\n";
            return 1;
        }
        const auto decoded = Traits::decode(*value);
        if (!decoded) {
            std::cerr << "encoded value out of range\n";
            return 1;
        }
        std::cout << Traits::to_string(*decoded) << '\n';
        return 0;
    }

    if (command == "check") {
        if (!Traits::is_valid_format(argument)) {
            std::cout << "format: invalid\n";
            std::cout << "check_digit: invalid\n";
            return 1;
        }

        std::cout << "format: valid\n";
        std::cout << "check_digit: "
                  << (Traits::has_valid_check_digit(argument) ? "valid" : "invalid")
                  << '\n';
        return Traits::has_valid_check_digit(argument) ? 0 : 1;
    }

    if (command == "check-digit") {
        const auto check_digit = Traits::calculate_check_digit(argument);
        if (!check_digit) {
            std::cerr << "invalid " << Traits::prefix_len << "-character " << Traits::name << " prefix\n";
            return 1;
        }
        std::cout << *check_digit << '\n';
        return 0;
    }

    print_identifier_cli_usage<Traits>(argv[0]);
    return 2;
}

} // namespace secids::detail

#endif
