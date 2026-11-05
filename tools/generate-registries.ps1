[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceRoot,

    [Parameter(Mandatory = $true)]
    [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'

function Get-SourceText([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Required source file was not found: $Path"
    }
    return Get-Content -LiteralPath $Path -Raw
}

function Get-UniqueStrings([string]$Text, [string]$Pattern) {
    $matches = [regex]::Matches($Text, $Pattern)
    $result = [System.Collections.Generic.List[string]]::new()
    $seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($match in $matches) {
        $value = $match.Groups[1].Value
        if ($seen.Add($value)) { [void]$result.Add($value) }
    }
    return @($result)
}

function Get-SafeId([string]$Value, [string]$TypeId, [hashtable]$UsedIds) {
    $trimmed = $Value.Trim()
    $slug = [regex]::Replace($trimmed.ToLowerInvariant(), '[^a-z0-9]+', '-')
    $slug = $slug.Trim('-')
    if ([string]::IsNullOrWhiteSpace($slug)) { $slug = 'unnamed' }
    $base = "$TypeId.$slug"
    $candidate = $base
    $suffix = 2
    while ($UsedIds.ContainsKey($candidate)) {
        $candidate = "$base-$suffix"
        $suffix++
    }
    $UsedIds[$candidate] = $true
    return $candidate
}

function Get-VariantGroup([string]$Text, [string]$GroupName) {
    $match = [regex]::Match($Text, "(?s)$([regex]::Escape($GroupName)).*?\};")
    if (-not $match.Success) { return '' }
    return $match.Value
}

function Get-FunctionBody([string]$Text, [string]$FunctionPattern) {
    $match = [regex]::Match($Text, $FunctionPattern)
    if (-not $match.Success) { return '' }
    $open = $Text.IndexOf('{', $match.Index)
    if ($open -lt 0) { return '' }
    $depth = 0
    for ($index = $open; $index -lt $Text.Length; $index++) {
        if ($Text[$index] -eq '{') { $depth++ }
        elseif ($Text[$index] -eq '}') {
            $depth--
            if ($depth -eq 0) { return $Text.Substring($match.Index, $index - $match.Index + 1) }
        }
    }
    return ''
}

$transformationSpecs = @(
    @{ id = 'appoggiatura'; name = 'Appoggiatura'; file = 'NewEnhancedAppoggiaturaTransformationMultipleUserChoice.cpp'; declaredFunction = 'generateRandomVariantPool\s*\('; directFunction = 'applyAppoggiatura\s*\(' },
    @{ id = 'compound-appoggiatura'; name = 'Compound appoggiatura'; file = 'NewEnhancedCompoundAppoggiaturaTransformationMultipleUserChoice.cpp'; declaredFunction = 'generateCompoundVariants\s*\('; directFunction = 'applyCompAppReg\s*\([^\{]+\)\s*\{' },
    @{ id = 'mordent'; name = 'Mordent'; file = 'NewEnhancedMordentTransformationMultipleUserChoice.cpp'; declaredFunction = 'applyMordentVariant\s*\('; directFunction = 'applyMordentVariants\s*\(' },
    @{ id = 'slide'; name = 'Slide'; file = 'NewEnhancedSlidesTransformationMultipleUserChoice.cpp'; declaredFunction = 'generateRandomVariantPool\s*\('; directFunction = 'applySlideVariants\s*\(' },
    @{ id = 'snap'; name = 'Snap'; file = 'NewEnhancedSnapTransformationMultipleUserChoice.cpp'; declaredFunction = 'applySnapVariant\s*\('; directFunction = 'applySnapVariants\s*\(' },
    @{ id = 'trill'; name = 'Trill'; file = 'NewEnhancedTrillTransformationMultipleUserChoice.cpp'; declaredFunction = 'generateRandomVariantPool\s*\('; directFunction = 'applyTrill\s*\(' },
    @{ id = 'turn'; name = 'Turn'; file = 'NewEnhancedTurnsTransformationMultipleUserChoice.cpp'; declaredFunction = 'generateRandomVariantPool\s*\('; directFunction = 'applyTurnVariants\s*\(' }
)

$variantTypes = [System.Collections.Generic.List[object]]::new()
foreach ($spec in $transformationSpecs) {
    $path = Join-Path (Join-Path $SourceRoot 'transformations') $spec.file
    $text = Get-SourceText $path
    $directBody = Get-FunctionBody $text $spec.directFunction
    $declaredBody = Get-FunctionBody $text $spec.declaredFunction
    $direct = @(Get-UniqueStrings $directBody 'variant\s*==\s*"([^"]*)"')
    if ($spec.id -eq 'compound-appoggiatura') {
        $declared = @(Get-UniqueStrings $declaredBody '\{\s*"([^"]+)"\s*,\s*"[^"]*"\s*\}')
    } else {
        $declared = @(Get-UniqueStrings $declaredBody '"([^"]+)"')
    }

    $names = [System.Collections.Generic.List[string]]::new()
    foreach ($name in @($declared + $direct)) {
        if (-not $names.Contains($name)) { [void]$names.Add($name) }
    }
    $usedIds = @{}
    $variants = [System.Collections.Generic.List[object]]::new()
    foreach ($name in $names) {
        $isDeclared = $declared -contains $name
        $isDirect = $direct -contains $name
        $state = if ($isDirect) { 'reference-only' } elseif ($isDeclared) { 'declared-no-direct-branch' } else { 'implementation-not-selectable' }
        $variants.Add([ordered]@{
            id = Get-SafeId $name $spec.id $usedIds
            displayName = $name
            canonicalName = $name.Trim()
            state = $state
            selectable = $isDeclared
            supportsRandomSelection = $isDeclared
        })
    }
    $variantTypes.Add([ordered]@{
        id = $spec.id
        displayName = $spec.name
        sourceFile = "reference/source/transformations/$($spec.file)"
        declaredSelectableCount = $declared.Count
        directImplementationBranchCount = $direct.Count
        variants = @($variants)
    })
}

$analyzerPath = Join-Path (Join-Path $SourceRoot 'analyzer') 'NewFixedMidiFileAnalyzer.cpp'
$analyzerText = Get-SourceText $analyzerPath
$labelMatch = [regex]::Match($analyzerText, '(?s)allLabels\s*=\s*\{(.*?)\};')
if (-not $labelMatch.Success) { throw 'Could not find the analyzer label catalog.' }
$labels = @(Get-UniqueStrings $labelMatch.Groups[1].Value '"([^"]+)"')
$processMatch = [regex]::Match($analyzerText, '(?s)void\s+processLabels\b.*?(?=\n\s*int\s+main\b)')
$processBody = if ($processMatch.Success) { $processMatch.Value } else { '' }
$labelDescriptors = [System.Collections.Generic.List[object]]::new()
$number = 1
foreach ($label in $labels) {
    $implementedInReference = $processBody.Contains('"' + $label + '"')
    $labelDescriptors.Add([ordered]@{
        number = $number
        id = $label
        state = if ($implementedInReference) { 'reference-implemented-not-integrated' } else { 'placeholder-no-detection-method' }
        detectionMethodId = if ($implementedInReference) { "reference.processLabels.$label" } else { "unimplemented.$label" }
        sourceFile = 'reference/source/analyzer/NewFixedMidiFileAnalyzer.cpp'
        note = if ($implementedInReference) { 'Reference analyzer contains a detection rule; integration and characterization are pending.' } else { 'Label is catalogued but no detection rule was found in processLabels.' }
    })
    $number++
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$variantDocument = [ordered]@{
    schemaVersion = 1
    generatedFrom = 'WorkingSourceFiles.zip'
    sourceRoot = 'reference/source/transformations'
    transformationTypes = @($variantTypes)
}
$labelDocument = [ordered]@{
    schemaVersion = 1
    generatedFrom = 'WorkingSourceFiles.zip'
    sourceFile = 'reference/source/analyzer/NewFixedMidiFileAnalyzer.cpp'
    labelCount = $labels.Count
    analysisLabels = @($labelDescriptors)
}
$variantDocument | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $OutputDirectory 'variant_registry.json') -Encoding utf8
$labelDocument | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $OutputDirectory 'analysis_label_registry.json') -Encoding utf8
Write-Output "Generated $($variantTypes.Count) transformation types and $($labels.Count) analysis labels."
