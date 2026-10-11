# GC-PROJEKT-RED: SSE/SIMD math library

Single-precision (float) vector and matrix math in C++20, with a scalar reference implementation (`Base_*`) and an SSE intrinsics version, plus a benchmark app.

## Requirements
- Windows x64, Visual Studio (toolset v145 as we used VS26), C++20
- CPU with SSE4.1 (the code uses `_mm_dp_ps`; ref. Limitations)

## Build
1. Open `GC_PROJEKT_RED/MathsLib/MathStarter.sln`
2. Select **Release | x64**, build
3. The executable is `MathsLib/bin/x64/Release/MathsLib.exe`

We set `/arch:AVX2` as an instruction set flag. We had originally planned to use AVX2 for our matrix calculations, but ended up not using the intrinsics methods.
And as we did our benchmark with the flag, we decided to let it set in the final report.

## Limitations
- `_mm_dp_ps` is SSE4.1 and `_mm_hadd_ps` is SSE3, beyond the SSE/SSE2 baseline (`Dot` function for Vec4 is still available without `_mm_dp_ps` (`DotManual`), but we chose not to keep the latter as it was slower in our benchmarks (ref. `Vec4.f` `l.51 - l.66` for detailed explanations))
- Single CPU tested (Ryzen 7 7700) -- Therefor results may vary depending on CPU used.

## Sources
Intel Intrinsics Guide, nanobench.

## AI usage
Claude was used for: explanations of SSE intrinsics, 
the benchmark protocol and the `run_bench.ps1` script (written by the AI),
and interpretation hypotheses for the results. 
