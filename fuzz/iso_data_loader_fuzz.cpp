#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>

#include "secids/runtime/iso_data_loader.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0) {
        return 0;
    }

    const auto path = std::filesystem::temp_directory_path() / "secids_iso_data_loader_fuzz_input";
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<const char*>(data + 1), static_cast<std::streamsize>(size - 1));
    }

    try {
        switch (data[0] % 3U) {
            case 0:
                static_cast<void>(secids::runtime::load_iso3166_country_csv(path));
                break;
            case 1:
                static_cast<void>(secids::runtime::load_iso4217_currency_csv(path));
                break;
            default:
                static_cast<void>(secids::runtime::load_iso4217_currency_json(path));
                break;
        }
    } catch (const std::exception&) {
        // Rejected input is expected; sanitizers detect memory-safety failures.
    }
    return 0;
}
