# F-Zero: Maximum Velocity Recomp - Release Packaging Script
param(
    [string]$Version = "0.1.0-preview-rc1"
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path "$PSScriptRoot\..").Path
$buildDir = Join-Path $projectRoot "build\Release"
$distDir = Join-Path $projectRoot "dist"
$packageName = "FZeroMaximumVelocityRecomp-windows-x64-v$Version"
$stagingRoot = Join-Path $distDir "staging"
$stageDir = Join-Path $stagingRoot $packageName
$zipPath = Join-Path $distDir "$packageName.zip"

Write-Host "============================================================"
Write-Host " Packaging F-Zero: Maximum Velocity Recomp"
Write-Host " Version: $Version"
Write-Host " Target:  $zipPath"
Write-Host "============================================================"

# Ensure output directories exist and clean previous staging
if (Test-Path $stageDir) {
    Remove-Item -Recurse -Force $stageDir
}
if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}
New-Item -ItemType Directory -Path $stageDir -Force | Out-Null
New-Item -ItemType Directory -Path $distDir -Force | Out-Null

# 1. Main Executable
$exePath = Join-Path $buildDir "FZeroMaximumVelocityRecomp.exe"
if (-not (Test-Path $exePath)) {
    throw "Release executable not found: $exePath. Ensure project is built with MSVC Release configuration."
}
Copy-Item -LiteralPath $exePath -Destination (Join-Path $stageDir "FZeroMaximumVelocityRecomp.exe") -Force
Write-Host "[+] Copied FZeroMaximumVelocityRecomp.exe ($((Get-Item $exePath).Length) bytes)"

# 2. SDL2.dll
$sdl2Path = Join-Path $buildDir "SDL2.dll"
if (-not (Test-Path $sdl2Path)) {
    throw "SDL2.dll not found in $buildDir"
}
Copy-Item -LiteralPath $sdl2Path -Destination (Join-Path $stageDir "SDL2.dll") -Force
Write-Host "[+] Copied SDL2.dll"

# 3. Microsoft Visual C++ Runtime (app-local deployment for x64)
$vcRedistCandidates = @(
    "C:\MYAPPLY\Microsoft Visual Studio\VC\Redist\MSVC\14.51.36231\x64\Microsoft.VC145.CRT",
    "C:\MYAPPLY\Microsoft Visual Studio\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT"
)
if ($env:VCToolsRedistDir) {
    $envCandidate = Join-Path $env:VCToolsRedistDir "x64\Microsoft.VC143.CRT"
    if (Test-Path $envCandidate) { $vcRedistCandidates = @($envCandidate) + $vcRedistCandidates }
    $envCandidate5 = Join-Path $env:VCToolsRedistDir "x64\Microsoft.VC145.CRT"
    if (Test-Path $envCandidate5) { $vcRedistCandidates = @($envCandidate5) + $vcRedistCandidates }
}

$vcDlls = @("msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll")
$vcRedistFound = $false
foreach ($cand in $vcRedistCandidates) {
    if (Test-Path $cand) {
        $allExist = $true
        foreach ($dll in $vcDlls) {
            if (-not (Test-Path (Join-Path $cand $dll))) { $allExist = $false; break }
        }
        if ($allExist) {
            foreach ($dll in $vcDlls) {
                Copy-Item -LiteralPath (Join-Path $cand $dll) -Destination (Join-Path $stageDir $dll) -Force
                Write-Host "[+] Copied VC++ runtime: $dll (from $cand)"
            }
            $vcRedistFound = $true
            break
        }
    }
}
if (-not $vcRedistFound) {
    Write-Warning "App-local VC++ runtime DLLs not found in standard redist paths. End users will require VC++ Redistributable installed."
}

# 4. recomp-ui Assets (fonts and img)
$assetsDir = Join-Path $buildDir "assets"
if (-not (Test-Path $assetsDir)) {
    throw "recomp-ui assets directory not found in $buildDir"
}
Copy-Item -LiteralPath $assetsDir -Destination $stageDir -Recurse -Force
Write-Host "[+] Copied recomp-ui assets directory (fonts & img)"

# 5. Game Configuration (Sanitized template)
$gameTomlSrc = Join-Path $projectRoot "game.toml"
if (-not (Test-Path $gameTomlSrc)) {
    throw "game.toml not found in $projectRoot"
}
Copy-Item -LiteralPath $gameTomlSrc -Destination (Join-Path $stageDir "game.toml") -Force
Write-Host "[+] Copied clean game.toml"

# 6. Documentation and Licenses
$docs = @("README.md", "RELEASE_NOTES.md", "LICENSE", "THIRD_PARTY_LICENSES.md")
foreach ($doc in $docs) {
    $docPath = Join-Path $projectRoot $doc
    if (-not (Test-Path $docPath)) {
        throw "Required document not found: $docPath"
    }
    Copy-Item -LiteralPath $docPath -Destination (Join-Path $stageDir $doc) -Force
    Write-Host "[+] Copied $doc"
}

# 7. Create empty saves placeholder directory
$stageSaves = Join-Path $stageDir "saves"
New-Item -ItemType Directory -Path $stageSaves -Force | Out-Null
Write-Host "[+] Initialized empty saves directory"

# 8. Strict Security & Cleanliness Verification
Write-Host "[*] Running security check for prohibited files..."
$prohibitedExts = @("*.gba", "*.bin", "*.sav", "*.pdb", "*.ilk", "*.obj", "*.lib", "*.cpp", "*.h", "*.log", "rom.cfg", "bios.cfg", "disc.cfg", "config.ini", "keybinds.ini")
$prohibitedFound = @()
foreach ($pattern in $prohibitedExts) {
    $found = Get-ChildItem -Path $stageDir -Filter $pattern -Recurse -File
    if ($found) {
        $prohibitedFound += $found
    }
}
if ($prohibitedFound.Count -gt 0) {
    $names = ($prohibitedFound | ForEach-Object { $_.FullName }) -join "`n  "
    throw "Security violation! Prohibited files detected in release package:`n  $names"
}
Write-Host "[+] Security check PASSED: No ROMs, BIOS, saves, PDBs, or private configs found."

# 9. Generate Checksums and Manifest
$files = Get-ChildItem -Path $stageDir -Recurse -File
$checksumLines = @()
$manifestFiles = @()

foreach ($f in $files) {
    $relative = $f.FullName.Substring($stageDir.Length + 1).Replace('\', '/')
    $hash = (Get-FileHash -LiteralPath $f.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    $checksumLines += "$hash  $relative"
    $manifestFiles += @{
        path = $relative
        size = $f.Length
        sha256 = $hash
    }
}

$checksumLines | Set-Content -LiteralPath (Join-Path $stageDir "SHA256SUMS.txt") -Encoding ASCII

$manifest = @{
    name = "FZeroMaximumVelocityRecomp"
    version = $Version
    platform = "windows-x64"
    package = "$packageName.zip"
    build_time = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    game = @{
        title = "F-Zero: Maximum Velocity"
        code = "AFZE"
        region = "USA, Europe"
        rom_sha1 = "8a08e29ec987f9cbdde21c34d5f7657aa7ba0be6"
        rom_crc32 = "BD5E9798"
        bios_sha1 = "300c20df6731a33952ded8c436f7f186d25d3492"
    }
    files = $manifestFiles
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $stageDir "manifest.json") -Encoding UTF8
Write-Host "[+] Generated SHA256SUMS.txt and manifest.json"

# 10. Create ZIP Archive
Write-Host "[*] Compressing release package to $zipPath ..."
# Use .NET System.IO.Compression for deterministic archive creation
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory($stageDir, $zipPath, [System.IO.Compression.CompressionLevel]::Optimal, $false)

$zipItem = Get-Item -LiteralPath $zipPath
$zipHash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()

Write-Host "============================================================"
Write-Host " Release Package Created Successfully!"
Write-Host " File:   $($zipItem.FullName)"
Write-Host " Size:   $($zipItem.Length) bytes ($([math]::Round($zipItem.Length / 1MB, 2)) MB)"
Write-Host " SHA256: $zipHash"
Write-Host "============================================================"
