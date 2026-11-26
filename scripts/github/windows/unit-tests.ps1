$ErrorActionPreference = "Stop"

ctest --test-dir build -L unit --no-tests=error --output-on-failure --parallel $env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    throw "Unit tests failed."
}

Write-Host "Unit tests passed."
