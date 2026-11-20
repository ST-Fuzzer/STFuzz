$ErrorActionPreference = "Stop"

$files = Get-ChildItem -Path dev, tests -Recurse -Filter *.cpp -File
if ($files.Count -eq 0) { Write-Host "No C++ files found."; exit 0 }

$srcDb = Join-Path "build" "compile_commands.json"
if (-not (Test-Path $srcDb)) { throw "Missing $srcDb" }

$tidyDir = Join-Path "build" "tidy-db"
New-Item -ItemType Directory -Force -Path $tidyDir | Out-Null

$mingwGpp = $null
if (Test-Path "C:\Qt\Tools") {
    $mingwGpp = Get-ChildItem "C:\Qt\Tools" -Filter "mingw*" -ErrorAction SilentlyContinue |
        Where-Object { $_.PSIsContainer } |
        ForEach-Object { Join-Path $_.FullName "bin\g++.exe" } |
        Where-Object { Test-Path $_ } |
        Select-Object -First 1
}
if (-not $mingwGpp) {
    $cmd = Get-Command g++.exe -ErrorAction SilentlyContinue
    if (-not $cmd) { $cmd = Get-Command x86_64-w64-mingw32-g++ -ErrorAction SilentlyContinue }
    if ($cmd) { $mingwGpp = $cmd.Source }
}

$db = (Get-Content -Path $srcDb -Raw) -replace '(\s*-mno-direct-extern-access\b|"-mno-direct-extern-access"\s*,?)', ''
if ($mingwGpp) {
    $mingwRoot = Split-Path (Split-Path $mingwGpp -Parent) -Parent
    $inc = Join-Path $mingwRoot "x86_64-w64-mingw32\include"
    if (-not (Test-Path $inc)) { $inc = Join-Path $mingwRoot "include" }
    if (Test-Path $inc) {
        $incEsc = ($inc -replace '\\', '\\\\')
        $db = $db -replace '(?i)(?:C:(?:/|\\+|\\\\))Program Files(?:/|\\+|\\\\)Microsoft Visual Studio(?:/|\\+|\\\\)[^"\\\s/]+(?:/|\\+|\\\\)VC(?:/|\\+|\\\\)Tools(?:/|\\+|\\\\)MSVC(?:/|\\+|\\\\)[^"\\\s/]+(?:/|\\+|\\\\)include', $incEsc
    }
}

[System.IO.File]::WriteAllText((Join-Path (Resolve-Path $tidyDir).Path "compile_commands.json"), $db)

$filter = '^(\.[/\\])?(dev|tests)[/\\]'
foreach ($file in $files) {
    Write-Host "Analyzing: $($file.FullName)"
    clang-tidy -p $tidyDir --header-filter=$filter $file.FullName
    if ($LASTEXITCODE -ne 0) { throw "Clang-tidy failed: $($file.Name)" }
}

Write-Host "Static analysis passed."
