#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "secids/runtime/iso_data_loader.hpp"

namespace {

void write_file(const std::filesystem::path& path, const char* text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

} // namespace

int main() {
    namespace fs = std::filesystem;
    using namespace secids::runtime;

    const fs::path tmp = fs::temp_directory_path() / "secids_iso_data_loader_test";
    fs::create_directories(tmp);

    const fs::path iso3166_csv = tmp / "iso3166.csv";
    write_file(
        iso3166_csv,
        "name,alpha-2,alpha-3,country-code,iso_3166-2,region,sub-region,intermediate-region,region-code,sub-region-code,intermediate-region-code\n"
        "\"United States of America\",US,USA,840,ISO 3166-2:US,Americas,Northern America,,019,021,\n"
        "\"Curaçao\",CW,CUW,531,ISO 3166-2:CW,Americas,Latin America and the Caribbean,Caribbean,019,419,029\n");

    const auto countries = load_iso3166_country_csv(iso3166_csv);
    assert(countries.size() == 2);
    assert(countries[0].alpha2 == "US");
    assert(countries[0].country_code == "840");
    assert(countries[1].name == "Curaçao");
    assert(countries[1].intermediate_region == "Caribbean");

    const fs::path iso4217_csv = tmp / "iso4217.csv";
    write_file(
        iso4217_csv,
        "Entity,Currency,AlphabeticCode,NumericCode,MinorUnit,WithdrawalDate\n"
        "AMERICAN SAMOA,US Dollar,USD,840,2,\n"
        "ZZ07_No_Currency,The codes assigned for transactions where no currency is involved,XXX,999,-,\n");

    const auto currencies_csv = load_iso4217_currency_csv(iso4217_csv);
    assert(currencies_csv.size() == 2);
    assert(currencies_csv[0].alphabetic_code == "USD");
    assert(currencies_csv[1].minor_unit == "-");

    const fs::path iso4217_json = tmp / "iso4217.json";
    write_file(
        iso4217_json,
        "{\n"
        "  \"USD\": {\n"
        "    \"name\": \"United States Dollar\",\n"
        "    \"demonym\": \"US\",\n"
        "    \"ISOnum\": 840,\n"
        "    \"symbol\": \"$\",\n"
        "    \"symbolNative\": \"$\",\n"
        "    \"majorSingle\": \"Dollar\",\n"
        "    \"majorPlural\": \"Dollars\",\n"
        "    \"minorSingle\": \"Cent\",\n"
        "    \"minorPlural\": \"Cents\",\n"
        "    \"ISOdigits\": 2,\n"
        "    \"decimals\": 2\n"
        "  },\n"
        "  \"JPY\": {\n"
        "    \"name\": \"Japanese Yen\",\n"
        "    \"demonym\": \"Japanese\",\n"
        "    \"ISOnum\": 392,\n"
        "    \"symbol\": \"\\u00a5\",\n"
        "    \"symbolNative\": \"\\u00a5\",\n"
        "    \"majorSingle\": \"Yen\",\n"
        "    \"majorPlural\": \"Yen\",\n"
        "    \"minorSingle\": \"Sen\",\n"
        "    \"minorPlural\": \"Sen\",\n"
        "    \"ISOdigits\": 0,\n"
        "    \"decimals\": 0\n"
        "  }\n"
        "}\n");

    const auto currencies_json = load_iso4217_currency_json(iso4217_json);
    assert(currencies_json.size() == 2);
    assert(currencies_json[0].alphabetic_code == "USD");
    assert(currencies_json[0].numeric_code.has_value() && *currencies_json[0].numeric_code == 840);
    assert(currencies_json[0].minor_unit.has_value() && *currencies_json[0].minor_unit == 2);
    assert(currencies_json[1].alphabetic_code == "JPY");
    assert(currencies_json[1].minor_unit.has_value() && *currencies_json[1].minor_unit == 0);

    fs::remove_all(tmp);

    std::cout << "secids_iso_data_loader_test passed\n";
    return 0;
}
