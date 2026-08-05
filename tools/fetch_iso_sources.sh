#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

mkdir -p "${ROOT_DIR}/data/iso3166" "${ROOT_DIR}/data/iso4217" "${ROOT_DIR}/data/iso4217-json"

curl -fL "https://raw.githubusercontent.com/lukes/ISO-3166-Countries-with-Regional-Codes/master/all/all.csv" \
  -o "${ROOT_DIR}/data/iso3166/all.csv"

curl -fL "https://raw.githubusercontent.com/datasets/currency-codes/main/data/codes-all.csv" \
  -o "${ROOT_DIR}/data/iso4217/codes-all.csv"

curl -fL "https://raw.githubusercontent.com/ourworldincode/currency/main/currencies.json" \
  -o "${ROOT_DIR}/data/iso4217-json/currencies.json"

python3 "${ROOT_DIR}/tools/generate_iso_source_manifest.py" \
  --retrieved-at "$(date -u +%Y-%m-%dT%H:%M:%SZ)"

printf 'Fetched ISO-3166 and ISO-4217 source data into %s/data\n' "${ROOT_DIR}"
