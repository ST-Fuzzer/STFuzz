#!/usr/bin/env bash
set -euo pipefail

ctest --test-dir build -L interface --no-tests=error --output-on-failure --parallel "$(nproc)"

echo "Interface tests passed."
