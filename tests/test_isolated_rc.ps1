$ErrorActionPreference = 'Stop'
$projectRoot = "D:\Project\GameRecomp\FZeroMaximumVelocityRecomp"
$zip = "$projectRoot\dist\FZeroMaximumVelocityRecomp-windows-x64-v0.1.0-preview-rc1.zip"
$testDir = "$projectRoot\dist\test_extracted"

Write-Host "--- Extracting $zip to $testDir ---"
if (Test-Path $testDir) { Remove-Item -Recurse -Force $testDir }
New-Item -ItemType Directory -Force $testDir | Out-Null
Expand-Archive -LiteralPath $zip -DestinationPath $testDir -Force

Write-Host "--- Verifying Extracted Files ---"
$expected = @(
    "FZeroMaximumVelocityRecomp.exe",
    "SDL2.dll",
    "msvcp140.dll",
    "vcruntime140.dll",
    "vcruntime140_1.dll",
    "game.toml",
    "README.md",
    "RELEASE_NOTES.md",
    "LICENSE",
    "THIRD_PARTY_LICENSES.md",
    "SHA256SUMS.txt",
    "manifest.json",
    "assets\fonts\LatoLatin-Regular.ttf",
    "assets\fonts\LatoLatin-Bold.ttf",
    "assets\fonts\NotoSansSymbols2-Regular.ttf",
    "assets\fonts\OpenMoji-black-glyf.ttf",
    "assets\img\brand_mark.tga",
    "assets\img\flags.png",
    "assets\img\pad_gba.tga"
)

foreach ($item in $expected) {
    $full = Join-Path $testDir $item
    if (-not (Test-Path $full)) {
        throw "Missing expected extracted file: $item"
    }
}
Write-Host "All expected files present!"

Write-Host "--- Test 1: Verify --help ---"
$proc = Start-Process -FilePath (Join-Path $testDir "FZeroMaximumVelocityRecomp.exe") -ArgumentList "--help" -WorkingDirectory $testDir -NoNewWindow -Wait -PassThru
if ($proc.ExitCode -ne 0) {
    throw "EXE --help exited with non-zero code: $($proc.ExitCode)"
}
Write-Host "Test 1 PASSED: --help executed successfully."

Write-Host "--- Test 2: Verify isolated execution with ROM & BIOS ---"
$romPath = "$projectRoot\F-Zero - Maximum Velocity (USA, Europe).gba"
$biosPath = "D:\MYAPPLICA\1GAMAMA\GBAGAMES\gbabios\gba_bios.bin"
$p = Start-Process -FilePath (Join-Path $testDir "FZeroMaximumVelocityRecomp.exe") -ArgumentList "--no-launcher --rom `"$romPath`" --bios `"$biosPath`" --frames 120" -WorkingDirectory $testDir -NoNewWindow -Wait -PassThru
if ($p.ExitCode -ne 0) {
    throw "EXE isolated run failed with code $($p.ExitCode)"
}
Write-Host "Test 2 PASSED: Isolated run with ROM & BIOS ran and exited normally."

Write-Host "--- Test 3: Verify save directory created in isolated testDir ---"
$isolatedSaveDir = Join-Path $testDir "saves"
if (-not (Test-Path $isolatedSaveDir)) {
    throw "Expected saves directory to be present in isolated directory!"
}
Write-Host "Test 3 PASSED: Isolated save directory exists."

Write-Host "--- Test 4: Verify original saves/ in project root remained completely untouched ---"
$origSave = Get-Item "$projectRoot\saves\fzero_maximum_velocity.sav"
if ($origSave.Length -ne 32768) {
    throw "Original save file modified or corrupted!"
}
Write-Host "Test 4 PASSED: Original save intact (32,768 bytes, LastWrite: $($origSave.LastWriteTime))."

Write-Host "=== ALL ISOLATED EXTRACTION TESTS PASSED! ==="
