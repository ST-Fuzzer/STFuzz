$ErrorActionPreference = "Stop"

$dist = "dist"
$archive = "Windows-Application.zip"

if (!(Test-Path $dist)) {
    throw "Deployment directory not found."
}

$files = Get-ChildItem $dist -Recurse -File

if ($files.Count -eq 0) {
    throw "Deployment directory is empty."
}

if (Test-Path $archive) {
    Remove-Item $archive -Force
}

Compress-Archive `
    -Path "$dist/*" `
    -DestinationPath $archive `
    -CompressionLevel Optimal

if (!(Test-Path $archive)) {
    throw "Application packaging failed."
}

Write-Host "Created $archive"
