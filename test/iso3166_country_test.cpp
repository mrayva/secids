#include <cassert>
#include <iostream>
#include <string>

#include "secids/iso3166_country.hpp"

int main() {
    using namespace secids::iso3166_country;

    static_assert(country_count == 249);
    static_assert(pack_alpha2("US").has_value());
    static_assert(pack_alpha3("USA").has_value());
    static_assert(!pack_alpha2("U1").has_value());
    static_assert(!pack_alpha3("US1").has_value());

    const auto* us = find_by_alpha2("US");
    assert(us != nullptr);
    assert(us->name == "United States of America");
    assert(unpack_alpha2(us->alpha2) == "US");
    assert(unpack_alpha3(us->alpha3) == "USA");
    assert(format_numeric_code(us->country_code) == "840");
    assert(to_string(us->region_id) == "Americas");
    assert(to_string(us->sub_region_id) == "Northern America");
    assert(iso_3166_2_code(*us) == "ISO 3166-2:US");

    const auto* gb = find_by_alpha3("GBR");
    assert(gb != nullptr);
    assert(unpack_alpha2(gb->alpha2) == "GB");
    assert(format_numeric_code(gb->country_code) == "826");

    const auto* afghanistan = find_by_country_code(4);
    assert(afghanistan != nullptr);
    assert(unpack_alpha2(afghanistan->alpha2) == "AF");
    assert(format_numeric_code(afghanistan->country_code) == "004");

    const auto* aland = find_by_alpha2("AX");
    assert(aland != nullptr);
    assert(aland->name == "Åland Islands");

    const auto* missing = find_by_alpha2("ZZ");
    assert(missing == nullptr);
    assert(format_numeric_code(missing_code).empty());

    std::size_t europe_count = 0;
    for (const auto& country : countries) {
        if (country.region_id == region::europe) {
            ++europe_count;
        }
    }
    assert(europe_count > 40);

    std::cout << "secids_iso3166_country_test passed\n";
    return 0;
}
