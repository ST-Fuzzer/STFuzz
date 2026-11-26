$ErrorActionPreference = "Stop"

ctest --test-dir build -L interface --no-tests=error --output-on-failure --parallel $env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    throw "Interface tests failed."
}

Write-Host "Interface tests passed."
