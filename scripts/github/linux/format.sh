#!/usr/bin/env bash
set -euo pipefail

files=$(find dev tests -type f \( -name '*.cpp' -o -name '*.hpp' \) 2>/dev/null || true)

if [ -z "$files" ]; then
  echo "No C++ files found. Skipping format check."
  exit 0
fi

for file in $files; do
  echo "Checking: $file"
  clang-format-18 --dry-run --Werror "$file" || {
    echo "Formatting check failed: $file"
    exit 1
  }
done

echo "Formatting check passed."
