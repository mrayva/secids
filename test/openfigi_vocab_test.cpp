#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "secids/openfigi/vocab_loader.hpp"
#include "secids/openfigi/vocab_registry.hpp"

namespace {

void write_file(const std::filesystem::path& path, const char* text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

} // namespace

int main() {
    namespace fs = std::filesystem;
    using namespace secids::openfigi;

    if (!vocab_snapshot::available()) {
        std::cout << "secids_openfigi_vocab_test skipped (string_bimap/pthash unavailable)\n";
        return 0;
    }

    const fs::path tmp = fs::temp_directory_path() / "secids_openfigi_vocab_test";
    fs::remove_all(tmp);
    fs::create_directories(tmp / "raw");
    fs::create_directories(tmp / "snapshots");

    write_file(tmp / "raw" / "micCode.json", "[\"XNYS\", \"XNAS\", \"XNYS\", \"ARCX\"]\n");

    auto snapshot = build_snapshot_from_raw_directory(tmp / "raw", vocab_domain::mic_code);
    assert(snapshot.size() == 3);
    assert(snapshot.find_id("XNAS").has_value());
    const auto arcx = snapshot.find_id("ARCX");
    assert(arcx.has_value());
    assert(snapshot.find_value(*arcx) == "ARCX");

    snapshot.save(snapshot_path(tmp / "snapshots", vocab_domain::mic_code));
    write_manifest_json(tmp / "snapshots");

    const auto registry = vocab_registry::load_from_directory(tmp / "snapshots");
    const auto* restored = registry.snapshot(vocab_domain::mic_code);
    assert(restored != nullptr);
    assert(restored->find_id("XNYS").has_value());
    assert(restored->find_value(*restored->find_id("XNAS")) == "XNAS");

    fs::remove_all(tmp);
    std::cout << "secids_openfigi_vocab_test passed\n";
    return 0;
}
