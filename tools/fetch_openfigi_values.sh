#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${1:-${ROOT_DIR}/data/openfigi}"

mkdir -p "${OUT_DIR}"

domains=(
  "micCode"
  "exchCode"
  "securityType"
  "marketSecDes"
  "securityType2"
)

for domain in "${domains[@]}"; do
  curl -L "https://api.openfigi.com/v3/mapping/values/${domain}" \
    -o "${OUT_DIR}/${domain}.json"
done

printf 'Fetched OpenFIGI value domains into %s\n' "${OUT_DIR}"
