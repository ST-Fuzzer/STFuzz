$ErrorActionPreference = "Stop"

$files = Get-ChildItem -Path dev, tests -Recurse -Filter *.cpp -File

if ($files.Count -eq 0) {
    Write-Host "No C++ files found. Skipping clang-tidy."
    exit 0
}

$sourceDb = Join-Path "build" "compile_commands.json"
if (-not (Test-Path $sourceDb)) {
    throw "Missing compile_commands.json at: $sourceDb"
}

$tidyDbDir = Join-Path "build" "tidy-db"
New-Item -ItemType Directory -Force -Path $tidyDbDir | Out-Null
$tidyDb = Join-Path $tidyDbDir "compile_commands.json"

$rawDb = Get-Content -Path $sourceDb -Raw
$cleanedDb = $rawDb -replace '\s*-mno-direct-extern-access\b', ''
[System.IO.File]::WriteAllText((Resolve-Path $tidyDbDir).Path + "\compile_commands.json", $cleanedDb)

foreach ($file in $files) {
    Write-Host "Analyzing: $($file.FullName)"

    clang-tidy -p $tidyDbDir $file.FullName

    if ($LASTEXITCODE -ne 0) {
        throw "Clang-tidy failed: $($file.Name)"
    }
}

Write-Host "Static analysis passed."
