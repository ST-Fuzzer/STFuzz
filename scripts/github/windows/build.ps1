$ErrorActionPreference = "Stop"

# Configure MSYS2 UCRT64 toolchain and Qt6 environment paths
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
$env:CMAKE_PREFIX_PATH = "C:\msys64\ucrt64"

cmake -S . -B build -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
    -DCMAKE_C_COMPILER=gcc `
    -DCMAKE_CXX_COMPILER=g++

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

cmake --build build --parallel $env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

Write-Host "Build completed successfully."
