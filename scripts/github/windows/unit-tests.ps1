$ErrorActionPreference = "Stop"

$test = "build/tests/unit/unit_test.exe"

if (!(Test-Path $test)) {
    throw "Unit test executable not found: $test"
}

& $test

if ($LASTEXITCODE -ne 0) {
    throw "Unit tests failed."
}

Write-Host "Unit tests passed."