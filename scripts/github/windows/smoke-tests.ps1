$ErrorActionPreference = "Stop"

ctest --test-dir build -L smoke --no-tests=error --output-on-failure --parallel $env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    throw "Smoke tests failed."
}

Write-Host "Smoke tests passed."
