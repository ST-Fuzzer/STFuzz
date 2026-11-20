$ErrorActionPreference = "Stop"

$mingw = "D:/a/STFuzz/Qt/Tools/mingw1310_64/bin/g++.exe"

if (!(Test-Path $mingw)) {
    throw "MinGW compiler not found: $mingw"
}

$files = Get-ChildItem -Path dev, tests -Recurse -Filter *.cpp -File

if ($files.Count -eq 0) {
    Write-Host "No C++ files found. Skipping clang-tidy."
    exit 0
}

foreach ($file in $files) {
    Write-Host "Analyzing: $($file.FullName)"

    clang-tidy `
        --query-driver="$mingw" `
        -p build `
        $file.FullName

    if ($LASTEXITCODE -ne 0) {
        throw "Clang-tidy failed: $($file.Name)"
    }
}

Write-Host "Static analysis passed."
