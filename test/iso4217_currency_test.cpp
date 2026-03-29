#include <cassert>
#include <iostream>
#include <string>

#include "secids/iso4217_currency.hpp"

int main() {
    using namespace secids::iso4217_currency;

    static_assert(currency_count == 178);
    static_assert(raw_source_row_count == 449);
    static_assert(current_source_row_count == 277);
    static_assert(pack_alphabetic_code("AAA").value() == 0);
    static_assert(pack_alphabetic_code("USD").has_value());
    static_assert(pack_alphabetic_code("usd").has_value());
    static_assert(!pack_alphabetic_code("US1").has_value());
    static_assert(!pack_alphabetic_code("US").has_value());

    const auto* usd = find_by_alphabetic_code("USD");
    assert(usd != nullptr);
    assert(usd->currency == "US Dollar");
    assert(usd->numeric_code == 840);
    assert(pack_alphabetic_code("USD").value() == usd->alphabetic_compact);
    assert(unpack_alphabetic_code(usd->alphabetic_compact) == "USD");
    assert(format_numeric_code(usd->numeric_code) == "840");
    assert(minor_unit_to_string(usd->minor_unit) == "2");

    const auto* eur = find_by_numeric_code(978);
    assert(eur != nullptr);
    assert(eur->alphabetic_code == "EUR");
    assert(eur->currency == "Euro");

    const auto* jpy = find_by_alphabetic_code("JPY");
    assert(jpy != nullptr);
    assert(jpy->minor_unit == 0);
    assert(minor_unit_to_string(jpy->minor_unit) == "0");

    const auto* clf = find_by_alphabetic_code("CLF");
    assert(clf != nullptr);
    assert(clf->minor_unit == 4);

    const auto* xxx = find_by_alphabetic_code("XXX");
    assert(xxx != nullptr);
    assert(xxx->numeric_code == 999);
    assert(xxx->minor_unit == variable_minor_unit);
    assert(minor_unit_to_string(xxx->minor_unit) == "-");

    const auto* xcg = find_by_alphabetic_code("XCG");
    assert(xcg != nullptr);
    assert(xcg->numeric_code == 532);

    const auto* zwg = find_by_alphabetic_code("ZWG");
    assert(zwg != nullptr);
    assert(zwg->numeric_code == 924);

    assert(find_by_alphabetic_code("BGN") == nullptr);
    assert(find_by_numeric_code(975) == nullptr);
    assert(find_by_alphabetic_code("ZZZ") == nullptr);
    assert(find_by_numeric_code(1000) == nullptr);
    assert(format_numeric_code(8) == "008");
    assert(format_numeric_code(missing_code).empty());
    assert(minor_unit_to_string(missing_minor_unit).empty());

    std::size_t x_prefix_count = 0;
    for (const auto& currency : currencies) {
        if (!currency.alphabetic_code.empty() && currency.alphabetic_code.front() == 'X') {
            ++x_prefix_count;
        }
    }
    assert(x_prefix_count > 10);

    std::cout << "secids_iso4217_currency_test passed\n";
    return 0;
}
