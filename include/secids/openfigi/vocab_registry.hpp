#ifndef SECIDS_OPENFIGI_VOCAB_REGISTRY_HPP
#define SECIDS_OPENFIGI_VOCAB_REGISTRY_HPP

#include <array>
#include <filesystem>
#include <optional>

#include "secids/openfigi/vocab_loader.hpp"

namespace secids::openfigi {

class vocab_registry {
public:
    static vocab_registry load_from_directory(const std::filesystem::path& root) {
        vocab_registry registry;
        for (std::size_t i = 0; i < all_vocab_domains.size(); ++i) {
            registry.entries_[i].domain = all_vocab_domains[i];
            const auto path = snapshot_path(root, all_vocab_domains[i]);
            if (std::filesystem::exists(path)) {
                registry.entries_[i].snapshot = vocab_snapshot::load(all_vocab_domains[i], path);
            }
        }
        return registry;
    }

    [[nodiscard]] const vocab_snapshot* snapshot(vocab_domain domain) const noexcept {
        for (const auto& entry : entries_) {
            if (entry.domain == domain && entry.snapshot.has_value()) {
                return &*entry.snapshot;
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::optional<std::uint32_t> find_id(vocab_domain domain, std::string_view value) const noexcept {
        const auto* view = snapshot(domain);
        if (view == nullptr) {
            return std::nullopt;
        }
        return view->find_id(value);
    }

    [[nodiscard]] std::string_view find_value(vocab_domain domain, std::uint32_t id) const noexcept {
        const auto* view = snapshot(domain);
        if (view == nullptr) {
            return {};
        }
        return view->find_value(id);
    }

private:
    struct entry {
        vocab_domain domain{};
        std::optional<vocab_snapshot> snapshot;
    };

    std::array<entry, all_vocab_domains.size()> entries_{};
};

} // namespace secids::openfigi

#endif
