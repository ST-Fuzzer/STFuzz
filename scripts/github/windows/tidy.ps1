$ErrorActionPreference = "Stop"

$tidyDir = "build/tidy-db"
New-Item -ItemType Directory -Force -Path $tidyDir | Out-Null

$db = (Get-Content "build/compile_commands.json" -Raw) -replace '"?-mno-direct-extern-access(=[^\s"]*)?"?,?\s*', ''
[System.IO.File]::WriteAllText("$PWD/$tidyDir/compile_commands.json", $db)

$gxx = (Get-Command g++).Source
$mingwRoot = Split-Path (Split-Path $gxx -Parent) -Parent

$root = $PWD.Path
$files = Get-ChildItem -Path dev, tests -Recurse -Filter *.cpp -File | ForEach-Object {
    ([IO.Path]::GetRelativePath($root, $_.FullName)) -replace '\\', '/'
}

$files | ForEach-Object -Parallel {
    $out = clang-tidy -p $using:tidyDir `
        --header-filter='^(\.[/\\])?(dev|tests)[/\\]' `
        --extra-arg-before=--target=x86_64-w64-mingw32 `
        --extra-arg-before="--gcc-toolchain=$using:mingwRoot" `
        --extra-arg-before=-fno-ms-compatibility `
        --extra-arg=-U_MSC_VER `
        --extra-arg=-Wno-unknown-warning-option `
        $_ 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "=== FAILED: $_ ==="
        $out | ForEach-Object { Write-Host $_ }
        throw "Failed: $_"
    }
} -ThrottleLimit ([Environment]::ProcessorCount)

Write-Host "Static analysis passed."
