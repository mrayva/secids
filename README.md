# secids

Header-only C++20 library for reversible ISIN and CUSIP text to `uint64_t` encoding.

It provides:
- structural ISIN validation
- structural CUSIP validation
- ISO 6166 check digit calculation and verification
- CUSIP check digit calculation and verification
- reversible `uint64_t` encoding/decoding
- small CLIs
- installable CMake package exports

## Why this fits in `uint64_t`

An ISIN has:
- 2 leading letters
- 9 alphanumeric payload characters
- 1 trailing decimal check digit

That value space is:

```text
26^2 * 36^9 * 10 = 4,738,381,338,321,616,896
```

which is below `2^64`.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Install

```bash
cmake --install build --prefix /tmp/secids-install
```

Installed artifacts:
- header: `include/secids/isin64.hpp`
- header: `include/secids/cusip64.hpp`
- CLI: `bin/secids_isin64_cli`
- CLI: `bin/secids_cusip64_cli`
- CMake package: `lib/cmake/secids`

## Library Usage

```cpp
#include <iostream>
#include "secids/isin64.hpp"

int main() {
    auto encoded = secids::isin64::encode_valid_isin("US0378331005");
    if (!encoded) {
        return 1;
    }

    auto decoded = secids::isin64::decode_isin(*encoded);
    if (!decoded) {
        return 1;
    }

    std::cout << *encoded << "\n";
    std::cout << secids::isin64::to_string(*decoded) << "\n";
}
```

## API

```cpp
std::optional<std::uint64_t> encode_isin(std::string_view isin);
std::optional<std::uint64_t> encode_valid_isin(std::string_view isin);
std::optional<std::array<char, 12>> decode_isin(std::uint64_t value);

bool is_valid_isin_format(std::string_view isin);
bool has_valid_check_digit(std::string_view isin);
bool is_valid_isin(std::string_view isin);
std::optional<int> calculate_check_digit(std::string_view first_11_chars);
std::string to_string(const decoded_type& isin);
```

```cpp
std::optional<std::uint64_t> encode_cusip(std::string_view cusip);
std::optional<std::uint64_t> encode_valid_cusip(std::string_view cusip);
std::optional<std::array<char, 9>> decode_cusip(std::uint64_t value);

bool is_valid_cusip_format(std::string_view cusip);
bool has_valid_check_digit(std::string_view cusip);
bool is_valid_cusip(std::string_view cusip);
std::optional<int> calculate_check_digit(std::string_view first_8_chars);
std::string to_string(const decoded_type& cusip);
```

Semantics:
- `encode_isin()`: requires valid ISIN character structure, does not require a correct check digit.
- `encode_valid_isin()`: requires full ISIN validity, including the check digit.
- `decode_isin()`: decodes any in-range packed value back to a 12-character ISIN-shaped string.
- `encode_cusip()`: requires valid CUSIP character structure, does not require a correct check digit.
- `encode_valid_cusip()`: requires full CUSIP validity, including the check digit.
- `decode_cusip()`: decodes any in-range packed value back to a 9-character CUSIP-shaped string.

## CLI

```bash
./build/secids_isin64_cli encode US0378331005
./build/secids_isin64_cli encode-strict US0378331005
./build/secids_isin64_cli decode 546395075064859685
./build/secids_isin64_cli check US0378331005
./build/secids_isin64_cli check-digit US037833100

./build/secids_cusip64_cli encode 037833100
./build/secids_cusip64_cli encode-strict 037833100
./build/secids_cusip64_cli decode 112064907630
./build/secids_cusip64_cli check 037833100
./build/secids_cusip64_cli check-digit 03783310
```

Commands:
- `encode <ISIN>`: encode any structurally valid ISIN text
- `encode-strict <ISIN>`: encode only if the check digit is valid
- `decode <UINT64>`: decode a packed value
- `check <ISIN>`: print format validity and check-digit validity
- `check-digit <11-char-prefix>`: compute the final check digit
- `secids_cusip64_cli` supports the same commands for CUSIPs with an 8-character prefix for `check-digit`

## CMake Consumer

After install:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES CXX)

find_package(secids CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE secids::isin64)
```

Configure with:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/tmp/secids-install
```

## Packaging

Binary/source archives can be produced with CPack:

```bash
cmake -S . -B build
cmake --build build
cpack --config build/CPackConfig.cmake
```

## vcpkg

The project includes a minimal `vcpkg.json` manifest. No external dependencies are required.

## Repository Setup

Local repository initialization:

```bash
git init
git add .
git commit -m "Initial commit"
```

GitHub publishing with GitHub CLI:

```bash
gh repo create secids --source=. --public --push
```

That command requires GitHub authentication in the local environment.

## License

MIT. See [LICENSE](LICENSE).
