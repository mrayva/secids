#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"${ROOT_DIR}/tools/fetch_iso_sources.sh"
python3 "${ROOT_DIR}/tools/generate_iso3166_country_header.py"
python3 "${ROOT_DIR}/tools/generate_iso4217_currency_header.py"

printf 'Fetched source data and regenerated ISO headers.\n'
