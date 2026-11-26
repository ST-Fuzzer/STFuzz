#!/usr/bin/env bash
set -euo pipefail

ctest --test-dir build -L smoke --no-tests=error --output-on-failure --parallel "$(nproc)"

echo "Smoke tests passed."
