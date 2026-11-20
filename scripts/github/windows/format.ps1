$ErrorActionPreference = "Stop"

$files = Get-ChildItem -Path dev, tests -Recurse -File |
    Where-Object { $_.Extension -in ".cpp", ".hpp" }

if ($files.Count -eq 0) {
    Write-Host "No C++ files found. Skipping format check."
    exit 0
}

foreach ($file in $files) {
    Write-Host "Checking: $($file.FullName)"

    clang-format --dry-run --Werror $file.FullName

    if ($LASTEXITCODE -ne 0) {
        throw "Formatting check failed: $($file.Name)"
    }
}

Write-Host "Formatting check passed."