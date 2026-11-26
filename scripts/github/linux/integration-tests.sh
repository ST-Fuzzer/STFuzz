#!/usr/bin/env bash
set -euo pipefail

ctest --test-dir build -L integration --no-tests=error --output-on-failure --parallel "$(nproc)"

echo "Integration tests passed."
