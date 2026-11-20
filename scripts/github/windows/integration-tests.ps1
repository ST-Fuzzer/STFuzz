$ErrorActionPreference = "Stop"

$test = "build/tests/int/int_test.exe"

if (!(Test-Path $test)) {
    throw "Integration test executable not found: $test"
}

& $test

if ($LASTEXITCODE -ne 0) {
    throw "Integration tests failed."
}

Write-Host "Integration tests passed."
