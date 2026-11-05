[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Label,

    [Parameter(Mandatory = $true)]
    [string]$BuildDirectory,

    [double]$MinimumFreeSpaceGB = 1
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$resolvedBuild = (Resolve-Path -LiteralPath $BuildDirectory).Path
$timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$safeLabel = [regex]::Replace($Label.Trim(), '[^A-Za-z0-9._-]+', '-')
$safeLabel = $safeLabel.Trim('-', '.')
if ([string]::IsNullOrWhiteSpace($safeLabel)) { throw 'The build label must contain at least one filename-safe character.' }

function Assert-ExternalDrive([string]$DriveLetter, [long]$RequiredBytes) {
    $volume = Get-Volume -DriveLetter $DriveLetter -ErrorAction Stop
    $root = "$DriveLetter`:\"
    if (-not (Test-Path -LiteralPath $root -PathType Container)) { throw "$DriveLetter`: is not mounted." }
    $probe = Join-Path $root (".etapki-write-probe-" + [guid]::NewGuid().ToString('N'))
    try {
        [System.IO.Directory]::CreateDirectory($probe) | Out-Null
    } catch {
        throw "$DriveLetter`: is not writable: $($_.Exception.Message)"
    } finally {
        if (Test-Path -LiteralPath $probe) { Remove-Item -LiteralPath $probe -Force -ErrorAction SilentlyContinue }
    }
    if ($volume.SizeRemaining -lt $RequiredBytes) {
        throw "$DriveLetter`: has only $([math]::Round($volume.SizeRemaining / 1GB, 2)) GB free; at least $([math]::Round($RequiredBytes / 1GB, 2)) GB is required."
    }
    if ($DriveLetter -eq 'D' -and $volume.FileSystem -ne 'FAT32') {
        Write-Warning "D: was expected to be FAT32 but reports $($volume.FileSystem)."
    }
    return $volume
}

function Get-TreeStats([string]$Path) {
    $files = @(Get-ChildItem -LiteralPath $Path -Recurse -File)
    $bytes = [long]($files | Measure-Object -Property Length -Sum).Sum
    [pscustomobject]@{ FileCount = $files.Count; TotalBytes = $bytes }
}

$eStage = Join-Path 'E:\ETAPKIPackages' "ETAPKI-$safeLabel-$timestamp"
$dStage = Join-Path 'D:\ETAPKIPackages' "ETAPKI-$safeLabel-$timestamp"
$eZip = "$eStage.zip"
$dZip = "$dStage.zip"
$createdEStage = $false
$createdDStage = $false

try {
    if ($resolvedBuild -notlike 'E:\*') { throw "BuildDirectory must be on E:, not $resolvedBuild" }
    $buildFiles = @('etapki_cli.exe', 'etapki_core_tests.exe', 'etapki_gui.exe')
    foreach ($file in $buildFiles) {
        if (-not (Test-Path -LiteralPath (Join-Path $resolvedBuild $file) -PathType Leaf)) {
            throw "Required build output is missing: $file"
        }
    }
    $estimatedBytes = [long]((Get-ChildItem -LiteralPath $resolvedBuild -Recurse -File | Measure-Object -Property Length -Sum).Sum) + 100MB
    $requiredBytes = [math]::Max($estimatedBytes * 2, [long]($MinimumFreeSpaceGB * 1GB))
    $eVolume = Assert-ExternalDrive 'E' $requiredBytes
    $dVolume = Assert-ExternalDrive 'D' $requiredBytes
    Write-Output "Expected temporary/package impact: approximately $([math]::Round($estimatedBytes / 1MB, 2)) MB per destination, plus ZIP overhead."
    if ($dVolume.FileSystem -eq 'FAT32') { Write-Output 'D: FAT32 check active; any generated file above 4 GB will fail.' }
    Write-Output 'Preflight passed; creating package staging directories.'

    New-Item -ItemType Directory -Path $eStage -Force | Out-Null
    $createdEStage = $true
    New-Item -ItemType Directory -Path $dStage -Force | Out-Null
    $createdDStage = $true

    foreach ($file in $buildFiles) { Copy-Item -LiteralPath (Join-Path $resolvedBuild $file) -Destination $eStage -Force }
    foreach ($file in @('AGENTS.md', 'README.md', 'PROJECT_PROPOSAL.md', 'CMakeLists.txt')) {
        Copy-Item -LiteralPath (Join-Path $repoRoot $file) -Destination $eStage -Force
    }
    Copy-Item -LiteralPath (Join-Path $repoRoot 'data') -Destination $eStage -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $repoRoot 'reference') -Destination $eStage -Recurse -Force
    Write-Output 'E: staging copy completed.'
    $eStats = Get-TreeStats $eStage
    if ($dVolume.FileSystem -eq 'FAT32' -and $eStats.TotalBytes -ge 4GB) { throw 'The generated package would exceed FAT32 per-file safety policy.' }
    foreach ($sourceFile in @(Get-ChildItem -LiteralPath $eStage -Recurse -File)) {
        $relative = $sourceFile.FullName.Substring($eStage.Length).TrimStart('\')
        $destinationFile = Join-Path $dStage $relative
        $destinationParent = Split-Path -Parent $destinationFile
        New-Item -ItemType Directory -Path $destinationParent -Force | Out-Null
        Copy-Item -LiteralPath $sourceFile.FullName -Destination $destinationFile -Force
    }
    Write-Output 'D: mirror copy completed.'
    $dStats = Get-TreeStats $dStage
    if ($eStats.FileCount -ne $dStats.FileCount -or $eStats.TotalBytes -ne $dStats.TotalBytes) {
        throw "D: mirror verification failed: E has $($eStats.FileCount) files/$($eStats.TotalBytes) bytes; D has $($dStats.FileCount) files/$($dStats.TotalBytes) bytes."
    }

    Compress-Archive -Path (Get-ChildItem -LiteralPath $eStage -Force | Select-Object -ExpandProperty FullName) -DestinationPath $eZip -CompressionLevel Optimal
    Write-Output 'E: ZIP creation completed.'
    if ((Get-Item -LiteralPath $eZip).Length -ge 4GB) { throw 'The E: ZIP exceeds the FAT32 per-file safety limit.' }
    Copy-Item -LiteralPath $eZip -Destination $dZip -Force
    if ((Get-Item -LiteralPath $eZip).Length -ne (Get-Item -LiteralPath $dZip).Length) { throw 'ZIP verification failed: byte sizes differ.' }

    Write-Output "E: package directory: $eStage ($([math]::Round($eStats.TotalBytes / 1MB, 2)) MB)"
    Write-Output "D: package directory: $dStage ($([math]::Round($dStats.TotalBytes / 1MB, 2)) MB)"
    Write-Output "E: package ZIP: $eZip ($([math]::Round((Get-Item -LiteralPath $eZip).Length / 1MB, 2)) MB)"
    Write-Output "D: package ZIP: $dZip ($([math]::Round((Get-Item -LiteralPath $dZip).Length / 1MB, 2)) MB)"
}
catch {
    if ($createdEStage -and (Test-Path -LiteralPath $eStage)) { Remove-Item -LiteralPath $eStage -Recurse -Force -ErrorAction SilentlyContinue }
    if ($createdDStage -and (Test-Path -LiteralPath $dStage)) { Remove-Item -LiteralPath $dStage -Recurse -Force -ErrorAction SilentlyContinue }
    Write-Error $_.Exception.Message
    exit 1
}
