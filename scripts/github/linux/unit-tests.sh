#!/usr/bin/env bash
set -euo pipefail

ctest --test-dir build -L unit --no-tests=error --output-on-failure --parallel "$(nproc)"

echo "Unit tests passed."
