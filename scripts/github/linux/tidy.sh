#!/usr/bin/env bash
set -euo pipefail

tidy_dir="build/tidy-db"
mkdir -p "$tidy_dir"

sed -E 's/ -mno-direct-extern-access(=[^ "]*)?//g' \
  build/compile_commands.json > "$tidy_dir/compile_commands.json"

find dev tests -type f -name '*.cpp' -print0 |
  xargs -0 -n1 -P "$(nproc)" bash -c '
    out=$(clang-tidy-18 -p build/tidy-db \
      --header-filter="^(\./)?(dev|tests)/" \
      --extra-arg=-Wno-unknown-warning-option "$0" 2>&1) || {
      echo "=== FAILED: $0 ==="
      echo "$out"
      exit 1
    }
  '

echo "Static analysis passed."
