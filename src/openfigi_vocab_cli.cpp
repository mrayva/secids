#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "secids/openfigi/vocab_loader.hpp"
#include "secids/openfigi/vocab_registry.hpp"

namespace {

void usage() {
    std::cerr
        << "Usage:\n"
        << "  secids_openfigi_vocab_cli build <domain> <raw-root> <out-root>\n"
        << "  secids_openfigi_vocab_cli lookup-id <domain> <snapshot-root> <value>\n"
        << "  secids_openfigi_vocab_cli lookup-value <domain> <snapshot-root> <id>\n"
        << "  secids_openfigi_vocab_cli stats <domain> <snapshot-root>\n";
}

} // namespace

int main(int argc, char** argv) {
    using namespace secids::openfigi;

    try {
        if (argc < 2) {
            usage();
            return 1;
        }

        const std::string command = argv[1];

        if (command == "build") {
            if (argc != 5) {
                usage();
                return 1;
            }
            const auto domain = parse_vocab_domain(argv[2]);
            if (!domain) {
                std::cerr << "unknown domain\n";
                return 1;
            }
            const std::filesystem::path raw_root = argv[3];
            const std::filesystem::path out_root = argv[4];
            std::filesystem::create_directories(out_root);
            auto snapshot = build_snapshot_from_raw_directory(raw_root, *domain);
            snapshot.save(snapshot_path(out_root, *domain));
            write_manifest_json(out_root);
            std::cout << "domain=" << to_string(*domain) << " size=" << snapshot.size() << "\n";
            return 0;
        }

        if (command == "lookup-id") {
            if (argc != 5) {
                usage();
                return 1;
            }
            const auto domain = parse_vocab_domain(argv[2]);
            if (!domain) {
                std::cerr << "unknown domain\n";
                return 1;
            }
            const auto registry = vocab_registry::load_from_directory(argv[3]);
            const auto id = registry.find_id(*domain, argv[4]);
            if (!id) {
                std::cout << "found=false\n";
                return 0;
            }
            std::cout << "found=true id=" << *id << "\n";
            return 0;
        }

        if (command == "lookup-value") {
            if (argc != 5) {
                usage();
                return 1;
            }
            const auto domain = parse_vocab_domain(argv[2]);
            if (!domain) {
                std::cerr << "unknown domain\n";
                return 1;
            }
            const auto registry = vocab_registry::load_from_directory(argv[3]);
            const auto value = registry.find_value(*domain, static_cast<std::uint32_t>(std::stoul(argv[4])));
            if (value.empty()) {
                std::cout << "found=false\n";
                return 0;
            }
            std::cout << "found=true value=" << value << "\n";
            return 0;
        }

        if (command == "stats") {
            if (argc != 4) {
                usage();
                return 1;
            }
            const auto domain = parse_vocab_domain(argv[2]);
            if (!domain) {
                std::cerr << "unknown domain\n";
                return 1;
            }
            const auto registry = vocab_registry::load_from_directory(argv[3]);
            const auto* snapshot = registry.snapshot(*domain);
            if (snapshot == nullptr) {
                std::cout << "available=false\n";
                return 0;
            }
            std::cout << "available=true size=" << snapshot->size() << "\n";
            return 0;
        }

        usage();
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }
}
