$ErrorActionPreference = "Stop"

choco install llvm -y
if ($LASTEXITCODE -ne 0) {
    throw "Failed to install LLVM."
}

$llvmBin = "C:\Program Files\LLVM\bin"
if ($env:GITHUB_PATH) {
    Add-Content -Path $env:GITHUB_PATH -Value $llvmBin
}
$env:PATH = "$llvmBin;$env:PATH"

Write-Host "LLVM installed successfully."
