$ErrorActionPreference = "Stop"

$tidyDir = "build/tidy-db"
New-Item -ItemType Directory -Force -Path $tidyDir | Out-Null

$db = (Get-Content "build/compile_commands.json" -Raw) -replace '"?-mno-direct-extern-access(=[^\s"]*)?"?,?\s*', ''
[System.IO.File]::WriteAllText("$PWD/$tidyDir/compile_commands.json", $db)

$root = $PWD.Path
$files = Get-ChildItem -Path dev, tests -Recurse -Filter *.cpp -File | ForEach-Object {
    ([IO.Path]::GetRelativePath($root, $_.FullName)) -replace '\\', '/'
}

$files | ForEach-Object -Parallel {
    clang-tidy -p $using:tidyDir --header-filter='^(\.[/\\])?(dev|tests)[/\\]' `
        --extra-arg-before=--target=x86_64-w64-mingw32 `
        --extra-arg-before=-fno-ms-compatibility `
        --extra-arg=-U_MSC_VER $_
    if ($LASTEXITCODE -ne 0) { throw "Failed: $_" }
} -ThrottleLimit ([Environment]::ProcessorCount)
