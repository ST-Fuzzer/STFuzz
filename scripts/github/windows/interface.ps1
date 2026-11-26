$ErrorActionPreference = "Stop"

$test = "build/tests/interface/interface_test.exe"

if (!(Test-Path $test)) {
    throw "Interface test executable not found: $test"
}

& $test

if ($LASTEXITCODE -ne 0) {
    throw "Interface tests failed."
}

Write-Host "Interface tests passed."
