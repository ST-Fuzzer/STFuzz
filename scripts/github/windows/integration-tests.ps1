$ErrorActionPreference = "Stop"

ctest --test-dir build -L integration --no-tests=error --output-on-failure --parallel $env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    throw "Integration tests failed."
}

Write-Host "Integration tests passed."
