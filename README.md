# secids

Header-only C++20 library for reversible ISIN, CUSIP, SEDOL, FIGI, and scoped RIC text to `uint64_t` encoding.

It provides:
- structural ISIN validation
- structural CUSIP validation
- structural SEDOL validation
- structural FIGI validation
- scoped RIC validation
- ISO 6166 check digit calculation and verification
- CUSIP check digit calculation and verification
- SEDOL check digit calculation and verification
- FIGI check digit calculation and verification
- RIC subset classification and validation
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
- header: `include/secids/secids.hpp`
- header: `include/secids/isin64.hpp`
- header: `include/secids/cusip64.hpp`
- header: `include/secids/sedol64.hpp`
- header: `include/secids/figi64.hpp`
- header: `include/secids/ric64.hpp`
- CLI: `bin/secids_isin64_cli`
- CLI: `bin/secids_cusip64_cli`
- CLI: `bin/secids_sedol64_cli`
- CLI: `bin/secids_figi64_cli`
- CLI: `bin/secids_ric64_cli`
- CMake package: `lib/cmake/secids`

## Library Usage

```cpp
#include <iostream>
#include "secids/secids.hpp"

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

```cpp
std::optional<std::uint64_t> encode_sedol(std::string_view sedol);
std::optional<std::uint64_t> encode_valid_sedol(std::string_view sedol);
std::optional<std::array<char, 7>> decode_sedol(std::uint64_t value);

bool is_valid_sedol_format(std::string_view sedol);
bool has_valid_check_digit(std::string_view sedol);
bool is_valid_sedol(std::string_view sedol);
std::optional<int> calculate_check_digit(std::string_view first_6_chars);
std::string to_string(const decoded_type& sedol);
```

```cpp
std::optional<std::uint64_t> encode_figi(std::string_view figi);
std::optional<std::uint64_t> encode_valid_figi(std::string_view figi);
std::optional<std::array<char, 12>> decode_figi(std::uint64_t value);

bool is_valid_figi_format(std::string_view figi);
bool has_valid_check_digit(std::string_view figi);
bool is_valid_figi(std::string_view figi);
std::optional<int> calculate_check_digit(std::string_view first_11_chars);
std::string to_string(const decoded_type& figi);
```

```cpp
std::optional<std::uint64_t> encode_ric(std::string_view ric);
std::optional<decoded_ric> decode_ric(std::uint64_t value);

bool is_valid_ric_format(std::string_view ric);
bool is_equity_ric(std::string_view ric);
bool is_index_ric(std::string_view ric);
std::string to_string(const decoded_ric& ric);
```

Semantics:
- `encode_isin()`: requires valid ISIN character structure, does not require a correct check digit.
- `encode_valid_isin()`: requires full ISIN validity, including the check digit.
- `decode_isin()`: decodes any in-range packed value back to a 12-character ISIN-shaped string.
- `encode_cusip()`: requires valid CUSIP character structure, does not require a correct check digit.
- `encode_valid_cusip()`: requires full CUSIP validity, including the check digit.
- `decode_cusip()`: decodes any in-range packed value back to a 9-character CUSIP-shaped string.
- `encode_sedol()`: requires valid SEDOL character structure, including the no-vowels rule for the first 6 characters, but does not require a correct check digit.
- `encode_valid_sedol()`: requires full SEDOL validity, including the check digit.
- `decode_sedol()`: decodes any in-range packed value back to a 7-character SEDOL-shaped string.
- `encode_figi()`: requires valid FIGI structure, including the restricted provider prefix and fixed third character `G`, but does not require a correct check digit.
- `encode_valid_figi()`: requires full FIGI validity, including the check digit.
- `decode_figi()`: decodes any in-range packed value back to a 12-character FIGI-shaped string.
- `encode_ric()`: supports a scoped RIC subset only:
  - equity RICs: `ROOT.EX`, where `ROOT` is `A-Z{1,4}` and `EX` is `A-Z{1,2}`
  - index RICs: `.CODE`, where `CODE` is `A-Z0-9{1,4}`
- `decode_ric()`: decodes only that supported subset

RIC scope limits:
- supported: `IBM.N`, `AAPL.OQ`, `.DJI`, `.SPX`
- unsupported by design: broader/synthetic commodity, futures, and other non-subset RICs such as `CL`, `EUR=`, or longer exchange suffix forms

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

./build/secids_sedol64_cli encode 0263494
./build/secids_sedol64_cli encode-strict 0263494
./build/secids_sedol64_cli decode 20288044
./build/secids_sedol64_cli check 0263494
./build/secids_sedol64_cli check-digit 026349

./build/secids_figi64_cli encode BBG000BLNNV0
./build/secids_figi64_cli encode-strict BBG000BLNNV0
./build/secids_figi64_cli decode 97913140
./build/secids_figi64_cli check BBG000BLNNV0
./build/secids_figi64_cli check-digit BBG000BLNNV

./build/secids_ric64_cli encode IBM.N
./build/secids_ric64_cli decode 1338100802
./build/secids_ric64_cli check .DJI
```

Commands:
- `encode <ISIN>`: encode any structurally valid ISIN text
- `encode-strict <ISIN>`: encode only if the check digit is valid
- `decode <UINT64>`: decode a packed value
- `check <ISIN>`: print format validity and check-digit validity
- `check-digit <11-char-prefix>`: compute the final check digit
- `secids_cusip64_cli` supports the same commands for CUSIPs with an 8-character prefix for `check-digit`
- `secids_sedol64_cli` supports the same commands for SEDOLs with a 6-character prefix for `check-digit`
- `secids_figi64_cli` supports the same commands for FIGIs with an 11-character prefix for `check-digit`
- `secids_ric64_cli` supports `encode`, `decode`, and `check` for the supported equity/index RIC subset

## CMake Consumer

After install:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES CXX)

find_package(secids CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE secids::secids)
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
