#ifndef SECIDS_OPENFIGI_VOCAB_SNAPSHOT_HPP
#define SECIDS_OPENFIGI_VOCAB_SNAPSHOT_HPP

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "secids/openfigi/vocab_types.hpp"

#if __has_include("string_bimap/pthash_bimap.hpp")
#include "string_bimap/pthash_bimap.hpp"
#define SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP 1
#else
#define SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP 0
#endif

namespace secids::openfigi {

namespace detail {

inline std::string trim_ascii(std::string_view value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && static_cast<unsigned char>(value[begin]) <= 0x20U) {
        ++begin;
    }
    while (end > begin && static_cast<unsigned char>(value[end - 1]) <= 0x20U) {
        --end;
    }
    return std::string(value.substr(begin, end - begin));
}

inline std::vector<std::string> normalize_vocab_values(const std::vector<std::string>& values) {
    std::vector<std::string> out;
    out.reserve(values.size());
    for (const auto& value : values) {
        auto normalized = trim_ascii(value);
        if (!normalized.empty()) {
            out.push_back(std::move(normalized));
        }
    }

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

} // namespace detail

class vocab_snapshot {
public:
    using id_type = std::uint32_t;

    vocab_snapshot() = default;

    static constexpr bool available() noexcept {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        return string_bimap::PthashBimap::available();
#else
        return false;
#endif
    }

    static vocab_snapshot build(vocab_domain domain, const std::vector<std::string>& values) {
        vocab_snapshot snapshot;
        snapshot.domain_ = domain;
        snapshot.rebuild(values);
        return snapshot;
    }

    static vocab_snapshot load(vocab_domain domain, const std::filesystem::path& path) {
        vocab_snapshot snapshot;
        snapshot.domain_ = domain;
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        snapshot.bimap_ = string_bimap::PthashBimap::load(path.string());
        return snapshot;
#else
        (void)path;
        throw std::logic_error("secids::openfigi::vocab_snapshot requires string_bimap");
#endif
    }

    void rebuild(const std::vector<std::string>& values) {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        const auto normalized = detail::normalize_vocab_values(values);
        bimap_.rebuild(normalized);
#else
        (void)values;
        throw std::logic_error("secids::openfigi::vocab_snapshot requires string_bimap");
#endif
    }

    void save(const std::filesystem::path& path) const {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        bimap_.save(path.string());
#else
        (void)path;
        throw std::logic_error("secids::openfigi::vocab_snapshot requires string_bimap");
#endif
    }

    [[nodiscard]] vocab_domain domain() const noexcept {
        return domain_;
    }

    [[nodiscard]] std::size_t size() const noexcept {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        return bimap_.size();
#else
        return 0;
#endif
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] std::optional<id_type> find_id(std::string_view value) const noexcept {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        return bimap_.find_as<id_type>(value);
#else
        (void)value;
        return std::nullopt;
#endif
    }

    [[nodiscard]] bool contains(std::string_view value) const noexcept {
        return find_id(value).has_value();
    }

    [[nodiscard]] std::string_view find_value(id_type id) const noexcept {
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
        return bimap_.by_id(id);
#else
        (void)id;
        return {};
#endif
    }

private:
    vocab_domain domain_ = vocab_domain::mic_code;
#if SECIDS_OPENFIGI_VOCAB_HAS_STRING_BIMAP
    string_bimap::PthashBimap bimap_;
#endif
};

} // namespace secids::openfigi

#endif
