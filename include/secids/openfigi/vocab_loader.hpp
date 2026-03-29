#ifndef SECIDS_OPENFIGI_VOCAB_LOADER_HPP
#define SECIDS_OPENFIGI_VOCAB_LOADER_HPP

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "secids/openfigi/vocab_snapshot.hpp"

namespace secids::openfigi {

inline std::vector<std::string> load_raw_values_json_array(const std::filesystem::path& path) {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
    return string_bimap::PthashBimap::load_values_from_json_array_file(path.string());
#else
    (void)path;
    throw std::logic_error("secids::openfigi::load_raw_values_json_array requires string_bimap");
#endif
}

inline std::vector<std::string> load_raw_values_csv_column(const std::filesystem::path& path,
                                                           std::string_view column_name) {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
    return string_bimap::PthashBimap::load_values_from_csv_file(path.string(), column_name);
#else
    (void)path;
    (void)column_name;
    throw std::logic_error("secids::openfigi::load_raw_values_csv_column requires string_bimap");
#endif
}

inline std::filesystem::path raw_values_path(const std::filesystem::path& root, vocab_domain domain) {
    return root / (std::string(to_string(domain)) + ".json");
}

inline std::filesystem::path snapshot_path(const std::filesystem::path& root, vocab_domain domain) {
    return root / (std::string(to_string(domain)) + ".snapshot.bin");
}

inline std::vector<std::string> load_domain_values(const std::filesystem::path& root, vocab_domain domain) {
    return load_raw_values_json_array(raw_values_path(root, domain));
}

inline vocab_snapshot build_snapshot_from_raw_directory(const std::filesystem::path& root, vocab_domain domain) {
    return vocab_snapshot::build(domain, load_domain_values(root, domain));
}

inline void write_manifest_json(const std::filesystem::path& root) {
    std::ofstream out(root / "manifest.json");
    if (!out) {
        throw std::runtime_error("failed to open manifest.json for writing");
    }

    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"domains\": [\n";
    for (std::size_t i = 0; i < all_vocab_domains.size(); ++i) {
        const auto domain = all_vocab_domains[i];
        out << "    {\"name\": \"" << to_string(domain) << "\", "
            << "\"raw\": \"" << raw_values_path(root, domain).filename().string() << "\", "
            << "\"snapshot\": \"" << snapshot_path(root, domain).filename().string() << "\"}";
        if (i + 1 < all_vocab_domains.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}

} // namespace secids::openfigi

#endif
