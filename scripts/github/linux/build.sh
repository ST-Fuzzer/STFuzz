#!/usr/bin/env bash
set -euo pipefail

command -v ninja >/dev/null || sudo apt-get install -y ninja-build

cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++

cmake --build build --parallel "$(nproc)"

echo "Build completed successfully."
