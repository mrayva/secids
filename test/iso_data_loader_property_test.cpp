#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "secids/runtime/iso_data_loader.hpp"

namespace {

void write_file(const std::filesystem::path& path, std::string_view content) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(content.data(), static_cast<std::streamsize>(content.size()));
}

void parse_json_without_crashing(const std::filesystem::path& path, std::string_view content) {
    write_file(path, content);
    try {
        const auto rows = secids::runtime::load_iso4217_currency_json(path);
        for (const auto& row : rows) {
            assert(!row.numeric_code || *row.numeric_code <= 999U);
        }
    } catch (const std::runtime_error&) {
    }
}

void parse_csv_without_crashing(const std::filesystem::path& path, std::string_view content) {
    write_file(path, content);
    try {
        static_cast<void>(secids::runtime::load_iso4217_currency_csv(path));
    } catch (const std::runtime_error&) {
    }
}

} // namespace

int main() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "secids_iso_data_loader_property_test";
    fs::create_directories(root);

    const std::string valid_json =
        "{\"USD\":{\"name\":\"Dollar\",\"ISOnum\":840,\"ISOdigits\":2,"
        "\"unknown\":[true,false,null,1.5e2,{\"x\":\"\\uD83D\\uDCB0\"}]}}";
    const std::string valid_csv =
        "Entity,Currency,AlphabeticCode,NumericCode,MinorUnit,WithdrawalDate\n"
        "UNITED STATES,US Dollar,USD,840,2,\n";

    for (std::size_t length = 0; length <= valid_json.size(); ++length) {
        parse_json_without_crashing(root / "input.json", std::string_view(valid_json).substr(0, length));
    }
    for (std::size_t i = 0; i < valid_json.size(); ++i) {
        std::string mutated = valid_json;
        mutated[i] = static_cast<char>(static_cast<unsigned char>(mutated[i]) ^ 0x7FU);
        parse_json_without_crashing(root / "input.json", mutated);
    }
    for (std::size_t i = 0; i < valid_csv.size(); ++i) {
        std::string mutated = valid_csv;
        mutated[i] = static_cast<char>(static_cast<unsigned char>(mutated[i]) ^ 0x55U);
        parse_csv_without_crashing(root / "input.csv", mutated);
    }

    fs::remove_all(root);
    std::cout << "secids_iso_data_loader_property_test passed\n";
    return 0;
}
