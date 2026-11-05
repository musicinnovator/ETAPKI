[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceRoot,

    [Parameter(Mandatory = $true)]
    [string]$ArchivePath,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) { throw "Source root not found: $SourceRoot" }
if (-not (Test-Path -LiteralPath $ArchivePath -PathType Leaf)) { throw "Archive not found: $ArchivePath" }

$files = @(Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -Filter '*.cpp' | Sort-Object FullName | ForEach-Object {
    $relative = $_.FullName.Substring($SourceRoot.Length).TrimStart('\') -replace '\\', '/'
    [ordered]@{
        path = $relative
        sizeBytes = $_.Length
        sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
})
$archiveHash = Get-FileHash -LiteralPath $ArchivePath -Algorithm SHA256
$document = [ordered]@{
    schemaVersion = 1
    archiveName = [System.IO.Path]::GetFileName($ArchivePath)
    archiveSha256 = $archiveHash.Hash.ToLowerInvariant()
    sourceRoot = 'reference/source'
    files = $files
}
New-Item -ItemType Directory -Path (Split-Path -Parent $OutputFile) -Force | Out-Null
$document | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputFile -Encoding utf8
Write-Output "Manifest written for $($files.Count) imported files."
