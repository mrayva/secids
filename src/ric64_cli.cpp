#include <iostream>
#include <string_view>

#include "secids/ric64.hpp"

namespace {

void print_usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " encode <RIC>\n"
        << "  " << argv0 << " decode <UINT64>\n"
        << "  " << argv0 << " check <RIC>\n";
}

std::optional<secids::ric64::value_type> parse_u64(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    secids::ric64::value_type value = 0;
    for (char c : text) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        const auto digit = static_cast<secids::ric64::value_type>(c - '0');
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
        const auto encoded = secids::ric64::encode_ric(argument);
        if (!encoded) {
            std::cerr << "unsupported RIC format\n";
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
        const auto decoded = secids::ric64::decode_ric(*value);
        if (!decoded) {
            std::cerr << "encoded value out of range\n";
            return 1;
        }
        std::cout << secids::ric64::to_string(*decoded) << '\n';
        return 0;
    }

    if (command == "check") {
        if (!secids::ric64::is_valid_ric_format(argument)) {
            std::cout << "format: invalid\n";
            std::cout << "kind: unsupported\n";
            return 1;
        }
        std::cout << "format: valid\n";
        std::cout << "kind: "
                  << (secids::ric64::is_equity_ric(argument) ? "equity" : "index")
                  << '\n';
        return 0;
    }

    print_usage(argv[0]);
    return 2;
}
