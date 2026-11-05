[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$RegistryPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile
)

$ErrorActionPreference = 'Stop'
$registry = Get-Content -LiteralPath $RegistryPath -Raw | ConvertFrom-Json
$typeMap = @{
    'appoggiatura' = 'Appoggiatura'
    'compound-appoggiatura' = 'CompoundAppoggiatura'
    'mordent' = 'Mordent'
    'slide' = 'Slide'
    'snap' = 'Snap'
    'trill' = 'Trill'
    'turn' = 'Turn'
}
$stateMap = @{
    'integrated' = 'Integrated'
    'reference-only' = 'ReferenceOnly'
    'declared-no-direct-branch' = 'Placeholder'
    'implementation-not-selectable' = 'Placeholder'
    'placeholder' = 'Placeholder'
    'blocked' = 'Blocked'
    'deprecated' = 'Deprecated'
}
function CppString([string]$Value) {
    $escaped = $Value.Replace('\', '\\').Replace('"', '\"').Replace("`r", '\r').Replace("`n", '\n')
    return '"' + $escaped + '"'
}
$lines = [System.Collections.Generic.List[string]]::new()
[void]$lines.Add('#include "etapki/core/generated_variant_data.h"')
[void]$lines.Add('')
[void]$lines.Add('namespace etapki::core {')
[void]$lines.Add('')
[void]$lines.Add('const std::vector<RegisteredVariant>& generatedVariantData() {')
[void]$lines.Add('    static const std::vector<RegisteredVariant> variants{')
foreach ($type in $registry.transformationTypes) {
    foreach ($variant in $type.variants) {
        $typeEnum = $typeMap[$type.id]
        $stateEnum = $stateMap[$variant.state]
        $selectable = if ($variant.selectable) { 'true' } else { 'false' }
        $random = if ($variant.supportsRandomSelection) { 'true' } else { 'false' }
        [void]$lines.Add(('        {{EmbellishmentTypeId::{0}, {1}, {2}, {3}, ImplementationState::{4}, {5}, {6}}},' -f $typeEnum,(CppString $variant.id),(CppString $variant.displayName),(CppString $variant.canonicalName),$stateEnum,$selectable,$random))
    }
}
[void]$lines.Add('    };')
[void]$lines.Add('    return variants;')
[void]$lines.Add('}')
[void]$lines.Add('')
[void]$lines.Add('} // namespace etapki::core')
New-Item -ItemType Directory -Path (Split-Path -Parent $OutputFile) -Force | Out-Null
$lines | Set-Content -LiteralPath $OutputFile -Encoding utf8
Write-Output "Generated $($registry.transformationTypes.Count) types and $((@($registry.transformationTypes.variants) | Measure-Object).Count) variant groups."
