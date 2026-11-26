#!/usr/bin/env bash
set -euo pipefail

want=18

if command -v clang-format-$want >/dev/null && command -v clang-tidy-$want >/dev/null; then
  echo "LLVM $want already installed"
  exit 0
fi

sudo apt-get update
sudo apt-get install -y clang-format-$want clang-tidy-$want

echo "LLVM installed successfully."
