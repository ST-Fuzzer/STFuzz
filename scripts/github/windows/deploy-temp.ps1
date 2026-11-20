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

$prevErrorAction = $ErrorActionPreference
$ErrorActionPreference = "Continue"
$out = & $windeployqt.Source `
    --release `
    --compiler-runtime `
    --dir $dist `
    $target 2>&1
$code = $LASTEXITCODE
$ErrorActionPreference = $prevErrorAction

$lines = @($out | ForEach-Object {
    if ($_ -is [System.Management.Automation.ErrorRecord]) {
        $_.Exception.Message
    } else {
        $_.ToString()
    }
})

$lines | ForEach-Object { Write-Host $_ }
$combined = $lines -join "`n"

if ($code -ne 0) {
    if ($combined -match "does not seem to be a Qt executable") {
        # Remove once stfuzz.exe links Qt and windeployqt exits 0 without warning.
        Write-Warning "Skipping windeployqt: $target has no Qt imports yet."
    } else {
        throw "Qt deployment failed with exit code $code."
    }
}

Write-Host "Deployment completed."
exit 0
