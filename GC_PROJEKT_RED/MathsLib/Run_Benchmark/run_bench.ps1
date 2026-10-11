<#
.SYNOPSIS
  Builds MathsLib (Release|x64), runs the benchmark N times with a fixed seed,
  saves the raw outputs and prints median / min / max plus reference/SIMD ratios.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\run_bench.ps1 -Seed 12345 -Runs 10
  powershell -ExecutionPolicy Bypass -File .\run_bench.ps1 -SkipBuild
#>
param(
    [uint32]$Seed = 12345,
    [int]$Runs = 10,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'

$root = 'C:\Users\mathp\OneDrive\Documents\GitHub\GC-PROJEKT-RED\GC_PROJEKT_RED\MathsLib'
$sln  = Join-Path $root 'MathStarter.sln'
$exe  = Join-Path $root 'bin\x64\Release\MathsLib.exe'

# ---------------------------------------------------------------- 1. MSBuild
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) { throw 'vswhere.exe not found: is Visual Studio installed?' }
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { throw 'MSBuild.exe not found' }

if (-not $SkipBuild) {
    Write-Host 'Building Release|x64 ...'
    & $msbuild $sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
}
if (-not (Test-Path $exe)) { throw "Executable not found: $exe" }

# ---------------------------------------------------------------- 2. Output folder
$stamp  = Get-Date -Format 'yyyy-MM-dd_HH-mm-ss'
$outDir = Join-Path $root "results\$stamp"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

# Child processes inherit this priority class.
try { (Get-Process -Id $PID).PriorityClass = 'High' } catch { Write-Warning 'Could not raise process priority' }

# ---------------------------------------------------------------- 3. Runs
Write-Host 'Warm-up run (discarded) ...'
$null = & $exe $Seed

$perRun = New-Object System.Collections.ArrayList
$header = $null
for ($i = 1; $i -le $Runs; $i++) {
    Write-Host ('Run {0}/{1}' -f $i, $Runs)
    $out = & $exe $Seed
    if ($LASTEXITCODE -ne 0) { throw "Run $i failed with exit code $LASTEXITCODE" }
    $out | Out-File -Encoding utf8 (Join-Path $outDir ('run_{0:D2}.txt' -f $i))

    if (-not $header) { $header = @($out | Where-Object { $_ -notmatch '^\|' }) }

    $map = [ordered]@{}
    foreach ($line in $out) {
        if ($line -match '^\|\s*([\d,]+(?:\.\d+)?)\s*\|.*?`([^`]+)`') {
            $map[$matches[2]] = [double]($matches[1] -replace ',', '')
        }
    }
    if ($map.Count -eq 0) { throw "Could not parse the nanobench table in run $i" }
    [void]$perRun.Add($map)
}

# ---------------------------------------------------------------- 4. Environment file
$cpuName = (Get-CimInstance Win32_Processor | Select-Object -First 1).Name.Trim()
$plan    = (powercfg /getactivescheme) -join ' '
$msbVer  = (& $msbuild -version -nologo | Select-Object -Last 1)
$commit  = 'n/a'
try {
    $c = & git -C $root rev-parse --short HEAD 2>$null
    if ($LASTEXITCODE -eq 0 -and $c) { $commit = $c }
} catch { }

$envLines = @(
    "Date         : $(Get-Date -Format 's')",
    "CPU          : $cpuName",
    "OS           : $([System.Environment]::OSVersion.VersionString)",
    "Power plan   : $plan",
    "MSBuild      : $msbVer",
    'Toolset      : v145 (from MathStarter.Demo.vcxproj)',
    'Flags        : /O2 /arch:AVX2 /fp:precise /std:c++20 /MD /W4 /WX /DNDEBUG  (keep in sync with the vcxproj)',
    "Git commit   : $commit",
    "Seed         : $Seed",
    "Runs         : $Runs (+1 discarded warm-up run)",
    'Process prio : High',
    '',
    '--- Program header (first run) ---'
) + $header
$envLines | Out-File -Encoding utf8 (Join-Path $outDir 'env.txt')

# ---------------------------------------------------------------- 5. Statistics
function Get-Stats([double[]]$v) {
    $s   = @($v | Sort-Object)
    $n   = $s.Count
    $mid = [int][math]::Floor($n / 2)
    if ($n % 2 -eq 1) { $med = $s[$mid] } else { $med = ($s[$mid - 1] + $s[$mid]) / 2 }
    [pscustomobject]@{ Median = $med; Min = $s[0]; Max = $s[$n - 1] }
}

$names = @($perRun[0].Keys)

$summary = foreach ($n in $names) {
    $vals = [double[]]@($perRun | ForEach-Object { $_[$n] })
    $st   = Get-Stats $vals
    [pscustomobject]@{
        Benchmark      = $n
        'Median ns/op' = [math]::Round($st.Median, 3)
        'Min'          = [math]::Round($st.Min, 3)
        'Max'          = [math]::Round($st.Max, 3)
        'Spread %'     = [math]::Round(100 * ($st.Max - $st.Min) / $st.Median, 1)
    }
}

$pairs = @(
    @{ Name = 'Normalize';      Simd = 'Normalize';      Ref = 'CPP_Normalize' },
    @{ Name = 'Dot';            Simd = 'Dot';            Ref = 'CPP_Dot' },
    @{ Name = 'Mat4x4 * Vec4f'; Simd = 'Mat4x4 * Vec4f'; Ref = 'CPP_Mat4x4 * Vec4f' }
)

# Ratio is computed run by run (reference time / SIMD time), then summarized.
$ratios = foreach ($p in $pairs) {
    if (($names -notcontains $p.Simd) -or ($names -notcontains $p.Ref)) { continue }
    $r  = [double[]]@($perRun | ForEach-Object { $_[$p.Ref] / $_[$p.Simd] })
    $st = Get-Stats $r
    if     ($st.Min -gt 1) { $verdict = 'SIMD faster in every run' }
    elseif ($st.Max -lt 1) { $verdict = 'SIMD slower in every run' }
    else                   { $verdict = 'ratio crosses 1.0: no clear difference' }
    [pscustomobject]@{
        Treatment      = $p.Name
        'Median ratio' = [math]::Round($st.Median, 2)
        'Min'          = [math]::Round($st.Min, 2)
        'Max'          = [math]::Round($st.Max, 2)
        Verdict        = $verdict
	'Count'        = '250 000'
    }
}

# ---------------------------------------------------------------- 6. Report
$sumText = ($summary | Format-Table -AutoSize | Out-String)
$ratText = ($ratios  | Format-Table -AutoSize | Out-String)

$report = "ns per vector (nanobench batch = vectors per batch), $Runs runs, seed $Seed`r`n" +
          $sumText + "`r`nRatio = reference time / SIMD time (>1 means SIMD faster)`r`n" + $ratText
$report | Out-File -Encoding utf8 (Join-Path $outDir 'summary.txt')
$summary | Export-Csv (Join-Path $outDir 'summary.csv') -NoTypeInformation -Encoding UTF8
$ratios  | Export-Csv (Join-Path $outDir 'ratios.csv')  -NoTypeInformation -Encoding UTF8

Write-Host ''
Write-Host $report
Write-Host "Raw outputs and summaries saved in: $outDir"
