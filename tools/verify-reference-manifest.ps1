[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ManifestPath,

    [Parameter(Mandatory = $true)]
    [string]$SourceRoot,

    [Parameter(Mandatory = $false)]
    [string]$ArchivePath
)

$ErrorActionPreference = 'Stop'
$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$failures = [System.Collections.Generic.List[string]]::new()
foreach ($file in $manifest.files) {
    $path = Join-Path $SourceRoot ($file.path -replace '/', '\')
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        [void]$failures.Add("Missing file: $($file.path)")
        continue
    }
    $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -ne $file.sha256 -or (Get-Item -LiteralPath $path).Length -ne $file.sizeBytes) {
        [void]$failures.Add("Changed file: $($file.path)")
    }
}
if ($ArchivePath) {
    $archiveHash = (Get-FileHash -LiteralPath $ArchivePath -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($archiveHash -ne $manifest.archiveSha256) { [void]$failures.Add("Changed source archive: $($manifest.archiveName)") }
}
if ($failures.Count -gt 0) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}
Write-Output "Reference manifest verified: $($manifest.files.Count) files; archive hash verified: $([bool]$ArchivePath)."
