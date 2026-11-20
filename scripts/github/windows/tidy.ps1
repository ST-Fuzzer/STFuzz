$ErrorActionPreference = "Stop"

$files = Get-ChildItem -Path dev, tests -Recurse -Filter *.cpp -File
if ($files.Count -eq 0) { Write-Host "No C++ files found."; exit 0 }

$srcDb = Join-Path "build" "compile_commands.json"
if (-not (Test-Path $srcDb)) { throw "Missing $srcDb" }

$tidyDir = Join-Path "build" "tidy-db"
New-Item -ItemType Directory -Force -Path $tidyDir | Out-Null

$db = (Get-Content -Path $srcDb -Raw) -replace '(\s*-mno-direct-extern-access\b|"-mno-direct-extern-access"\s*,?)', ''
[System.IO.File]::WriteAllText((Join-Path (Resolve-Path $tidyDir).Path "compile_commands.json"), $db)

$filter = '^(\.[/\\])?(dev|tests)[/\\]'
foreach ($file in $files) {
    Write-Host "Analyzing: $($file.FullName)"
    clang-tidy `
        -p $tidyDir `
        --header-filter=$filter `
        --extra-arg-before=--target=x86_64-w64-mingw32 `
        --extra-arg-before=-fno-ms-compatibility `
        --extra-arg=-U_MSC_VER `
        $file.FullName
    if ($LASTEXITCODE -ne 0) { throw "Clang-tidy failed: $($file.Name)" }
}

Write-Host "Static analysis passed."
