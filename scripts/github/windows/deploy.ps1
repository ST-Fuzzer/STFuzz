$ErrorActionPreference = "Stop"

$exe = "build/dev/src/app/stfuzz.exe"
$dist = "dist"

if (!(Test-Path $exe)) {
    throw "STFuzz executable not found: $exe"
}

$windeployqt = Get-Command windeployqt -ErrorAction Stop

if (Test-Path $dist) {
    Remove-Item $dist -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $dist | Out-Null

$target = Join-Path $dist "stfuzz.exe"

Copy-Item $exe $target

& $windeployqt.Source `
    --release `
    --compiler-runtime `
    --dir $dist `
    $target

if ($LASTEXITCODE -ne 0) {
    throw "Qt deployment failed."
}

Write-Host "Deployment completed."
