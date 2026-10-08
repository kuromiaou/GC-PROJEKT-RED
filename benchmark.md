Results can be found in "GC-PROJEKT-RED\GC_PROJEKT_RED\MathsLib\results"

ns per vector (nanobench batch = vectors per batch), 10 runs, seed 12345

Benchmark          Median ns/op  Min  Max Spread %
---------          ------------  ---  --- --------
Normalize                 1.845 1.66 2.01       19
CPP_Normalize              1.75 1.71 2.16     25.7
Dot                       0.805  0.8  1.1     37.3
CPP_Dot                     0.5 0.49 0.55       12
Mat4x4 * Vec4f             1.35 1.34  1.4      4.4
CPP_Mat4x4 * Vec4f         2.96 2.85 3.45     20.3

Ratio = reference time / SIMD time (>1 means SIMD faster)

Treatment      Median ratio  Min  Max Verdict                                Count
---------      ------------  ---  --- -------                                -----
Normalize              0.97 0.86 1.21 ratio crosses 1.0: no clear difference 100  
Dot                    0.61 0.45 0.67 SIMD slower in every run               100  
Mat4x4 * Vec4f         2.15 2.11 2.56 SIMD faster in every run               100

-------------------------------------------------------------------------------------
Dot: 32
CPU: AuthenticAMD / AMD Ryzen 7 7700 8-Core Processor
SSE usable: true
AVX2 usable: true
FMA usable: true
Seed: 12345, vectors per batch: 1000
Batch incl. checksum: min 0.002 ms, median 0.003 ms, max 0.003 ms
Observable checksum: 69.522

|           benchmark |               ns/op |     Min |       Max 
|--------------------:|--------------------:|--------:|-----------
|         `Normalize` |               1.76  |    1.63 |      2.05   
|     `CPP_Normalize` |               1.735 |    1.71 |      1.8  
|               `Dot` |               0.81  |     0.8 |      0.82
|           `CPP_Dot` |               0.49  |    0.49 |      0.55  
|    `Mat4x4 * Vec4f` |               1.385 |    1.37 |      1.43 
| `CPP_Mat4x4 * Vec4f`|               2.93  |    2.9  |      3

Ratio = reference time / SIMD time (>1 means SIMD faster)

Treatment      Median ratio  Min  Max Verdict                                Count
---------      ------------  ---  --- -------                                -----
Normalize                 1 0.83 1.09 ratio crosses 1.0: no clear difference 1 000
Dot                     0.6  0.6 0.69 SIMD slower in every run               1 000
Mat4x4 * Vec4f         2.12 2.05 2.16 SIMD faster in every run               1 000



-------------------------------------------------------------------------------------

Benchmark          Median ns/op  Min  Max Spread %
---------          ------------  ---  --- --------
Normalize                 1.945 1.68 2.03       18
CPP_Normalize             1.775 1.74 1.93     10.7
Dot                        0.81 0.79 0.91     14.8
CPP_Dot                     0.5 0.49 0.51        4
Mat4x4 * Vec4f             2.48 2.17 3.07     36.3
CPP_Mat4x4 * Vec4f         3.66 3.37 3.96     16.1

Ratio = reference time / SIMD time (>1 means SIMD faster)

Treatment      Median ratio  Min  Max Verdict                                Count  
---------      ------------  ---  --- -------                                -----  
Normalize              0.92 0.86 1.13 ratio crosses 1.0: no clear difference 250 000
Dot                    0.61 0.55 0.65 SIMD slower in every run               250 000
Mat4x4 * Vec4f         1.45 1.21 1.78 SIMD faster in every run               250 000

-------------------------------------------------------------------------------------

Benchmark          Median ns/op  Min  Max Spread %
---------          ------------  ---  --- --------
Normalize                  2.03  1.7 2.03     16.3
CPP_Normalize              1.77 1.76 1.79      1.7
Dot                        0.82 0.81 0.86      6.1
CPP_Dot                    0.53 0.52 0.77     47.2
Mat4x4 * Vec4f             3.48 3.43 3.56      3.7
CPP_Mat4x4 * Vec4f         4.21 4.13 4.46      7.8

Ratio = reference time / SIMD time (>1 means SIMD faster)

Treatment      Median ratio  Min  Max Verdict                                Count    
---------      ------------  ---  --- -------                                -----    
Normalize              0.87 0.87 1.05 ratio crosses 1.0: no clear difference 1 000 000
Dot                    0.65  0.6 0.94 SIMD slower in every run               1 000 000
Mat4x4 * Vec4f          1.2 1.19 1.28 SIMD faster in every run               1 000 000