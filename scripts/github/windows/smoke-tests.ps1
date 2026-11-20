$ErrorActionPreference = "Stop"

$test = "build/tests/smoke/smoke_test.exe"

if (!(Test-Path $test)) {
    throw "Smoke test executable not found: $test"
}

& $test

if ($LASTEXITCODE -ne 0) {
    throw "Smoke tests failed."
}

Write-Host "Smoke tests passed."