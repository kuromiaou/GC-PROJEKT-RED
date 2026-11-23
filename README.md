# GC-PROJEKT-RED

C++20 project developed over the year, made of several modules:

| Module | Folder | Content |
|---|---|---|
| Math library | `third_party/MathsLib` | SSE/SIMD vector and matrix math, with a benchmark |
| Engine | `Platform`, `App` | CPU 3D renderer, ECS, procedural terrain, UI |
| Asset Engine | `AssetEngine`, `Adapters`, `Tests`, `Demo` | binary asset archives: serialization, versioning, integrity checks, physics state save/restore |

## Requirements
- Windows x64, Visual Studio (toolset v145, as we used VS26), C++20
- CMake 3.20 or later
- CPU with SSE4.1 for the math library (the code uses `_mm_dp_ps`; ref. Limitations)
- The Asset Engine has no external dependency

## Build & Launch
1. Open `GC-PROJEKT-RED/GC_PROJEKT_RED/CMakeLists.txt`
2. Open the **PowerShell Terminal** in VS: `AZERTY = ctrl + ù` --- `QWERTY = ctrl + ²`
3. Run `cmake -S . -B build`
4. Run `cmake --build build --config Release`
5. Run the engine: `.\build\App\Release\App.exe`
6. Run the Asset Engine tests: `ctest --test-dir build -C Release --output-on-failure`
7. Run the Asset Engine demo (writes the example archives into `data/`): `.\build\Demo\Release\AssetDemo.exe data`

The tests can also be run one by one for a detailed output:

```
.\build\Tests\Release\AssetTests.exe
.\build\Tests\Release\AdapterTests.exe
```

We set `/arch:AVX2` as an instruction set flag. We had originally planned to use AVX2 for our matrix calculations, but ended up not using the intrinsics methods. As we ran our benchmark with the flag, we decided to keep it set in the final report.

---

# Math library

Single-precision (float) vector and matrix math, with a scalar reference implementation (`Base_*`) and an SSE intrinsics version, plus a benchmark app.

## Limitations
- `_mm_dp_ps` is SSE4.1 and `_mm_hadd_ps` is SSE3, beyond the SSE/SSE2 baseline. A `Dot` function for Vec4 is still available without `_mm_dp_ps` (`DotManual`), but we chose not to use it as it was slower in our benchmarks (ref. `Vec4f.h`, l.51 - l.66, for detailed explanations).
- Single CPU tested (Ryzen 7 7700): results may vary depending on the CPU used.

---

# Asset Engine

A reusable binary asset archiving library, independent from the engine, with adapters for two general-purpose types and for a physics scene. It saves structured data, loads it back, handles schema evolution and detects corrupted archives.

## Organisation

| Folder | Role | Depends on |
|---|---|---|
| `AssetEngine/` | library: `BinaryWriter`, `BinaryReader`, sequences, CRC-32, archive | nothing |
| `Adapters/` | adapters: `Player`, `Inventory` (general types) and `PhysicsWorld` | AssetEngine |
| `Tests/` | `AssetTests` (core only) and `AdapterTests` (adapters) | AssetEngine / Adapters |
| `Demo/` | `AssetDemo`: demo, example archives and measurements | Adapters |

```
AssetEngine  <-  Adapters  <-  AssetDemo
     ^              ^
 AssetTests    AdapterTests
```

Three layers are kept separate, as required: byte access (`BinaryWriter`, `BinaryReader`), the archive format (`Archive.h`), and the adapters for domain types. `AssetEngine` includes no header from the engine, SDL or the adapters: `AssetTests` builds and tests it on its own.

## API

```cpp
// Write an archive
ArchiveWriter writer;
writer.Add(1, asset_types::Player, Player{100, 5.5f, "Bob", 7});
writer.Add(10, asset_types::Inventory, Inventory{"Bob", {{1, 3}, {42, 1}}});
std::string error;
writer.SaveToFile("data/save.gast", error);

// Read: everything is validated when the archive is opened
ArchiveReader reader;
if (!reader.OpenFile("data/save.gast", error)) { /* error explains why */ }
for (const AssetEntry& e : reader.Entries()) { /* id, type, offset, size */ }

// Load a single asset by ID, without deserializing the others
Inventory inventory;
reader.Load(10, asset_types::Inventory, inventory, error);
```

**Adding a type** only requires two functions, without modifying the core:

```cpp
void Write(BinaryWriter& writer, const MyType& value);
bool Read(BinaryReader& reader, MyType& value);
```

`ArchiveWriter::Add` and `ArchiveReader::Load` find them automatically (argument-dependent lookup).

## Type encoding

All integers are **little-endian**, whatever the machine.

| Type | Encoding | Check on read |
|---|---|---|
| `uint8` / `uint32` / `uint64` | 1 / 4 / 8 bytes | enough bytes left |
| `int32` | 4 bytes, two's complement | same |
| `bool` | 1 byte: `0` or `1` | any other value is rejected |
| `float` | 32-bit IEEE 754 (`std::bit_cast` to `uint32`) | checked at compile time (`static_assert`) |
| `string` | `uint32` length + UTF-8 bytes | length <= 1 MiB and <= bytes left |
| sequence | `uint32` count + elements | count <= type maximum and <= bytes left / minimum element size |

No data is written by copying an object's memory: no `sizeof`, pointers, padding or raw containers. References between objects are **stable IDs** (`uint64`).

Rule shared by every read function: on failure, it returns `false`, does not modify the output value and does not consume any byte.

## Archive format (version 1)

```
offset 0    magic            4 bytes   "GAST"
offset 4    formatVersion    uint32    1
offset 8    assetCount       uint32
offset 12   directoryOffset  uint64
offset 20   crc32            uint32
offset 24   payloads         asset data, one after another
...         directory        assetCount entries of 28 bytes, up to the end of the file
```

Directory entry (28 bytes): `id` uint64, `type` uint32, `offset` uint64, `size` uint64.

The directory is placed **at the end**: when writing, all offsets are known once the payloads are written. An empty archive is 24 bytes long.

### Asset types

| Value | Type | Payload |
|---|---|---|
| 0 | forbidden | |
| 1 | `Player` | schema `uint32`, then health `uint32`, speed `float`, name `string`; V2 adds level `uint32` |
| 2 | `Inventory` | owner `string`, sequence of `Item` (id `uint32`, count `uint32`) |
| 3 | `PhysicsScene` | see "Physics integration" |
| 4 | `PhysicsSnapshot` | same payload as 3 |

## Integrity check: CRC-32

- **Algorithm**: standard CRC-32 (IEEE 802.3, as used by zlib and PNG), reflected polynomial `0xEDB88320`, initial value and final XOR `0xFFFFFFFF`. Check value: `CRC("123456789") = 0xCBF43926` (tested). Table-based implementation written by us (`Crc32.h`), no library.
- **Bytes covered**: the whole file **except** the CRC field itself, i.e. bytes `[0, 20)` (magic, version, asset count, directory offset) then `[24, end)` (payloads and directory). Metadata is therefore protected: a modified directory is detected.
- **Limits**: the CRC detects accidental corruption, not deliberate tampering (anyone can recompute a valid CRC). It guarantees **neither authenticity nor validity**: this is why the structural checks below run even when the CRC is correct. The tests check this by recomputing a valid CRC on deliberately inconsistent archives.

## Validation on open

`ArchiveReader::Open` rejects the archive, with a diagnostic message, if:

1. the file is shorter than 24 bytes;
2. the magic is not `GAST`;
3. the format version is not 1 (unknown versions are explicitly rejected);
4. the CRC does not match;
5. the asset count exceeds 1,000,000;
6. the directory does not fit exactly between `directoryOffset` and the end of the file;
7. an entry has type 0, a payload outside the `[24, directoryOffset)` area, or an ID already seen.

Ranges are checked without any possible overflow: `offset <= total && size <= total - offset`. A file larger than 1 GiB is rejected before any allocation. `Load` then checks the expected type, decodes into a temporary object, rejects trailing bytes, and only modifies the destination object on full success: never a partially loaded state.

On write, the archive is written to `file.tmp` and then renamed: a write failure leaves the previous file untouched.

## Compatibility and versioning

Two levels of versioning:

- **format version** (header): describes the archive structure. Only version 1 exists; any other is rejected.
- **schema version** (start of each `Player` and `PhysicsWorld` payload): describes the content of a type.

`Player` exists in two versions. V1 contains health, speed and name. V2 adds `level`. The current reader reads both: a V1 payload is loaded with **`level = 1`** (default value). An unknown schema version is rejected, never guessed. The archives `data/general_v1.gast` and `data/general_v2.gast` (written by the demo) contain examples of both versions.

## Physics integration

Our engine (CPU renderer + ECS) does not have a complete physics engine yet. As allowed by the brief, state restoration is demonstrated with the data structures of `Adapters/Physics.h`: `PhysicsWorld`, `RigidBody`, `Collider`, `DistanceJoint`.

**Saved data**: gravity, fixed timestep, step index, and for each body: ID, static or dynamic state, position, orientation, mass, inertia, linear and angular velocities, collider (none, sphere or box). Joints link two bodies **by their IDs**.

**Conventions**: SI units (metres, seconds, kilograms), Y axis up, orientation as a unit quaternion `(x, y, z, w)`, inertia = diagonal of the inertia tensor in local space, mass ignored for a static body.

**Encoding**: schema `uint32` (1), gravity 3 x `float`, fixedTimestep `float`, stepIndex `uint64`, sequence of bodies (94 bytes each), sequence of joints (20 bytes each).

**Restoration**: `Read` decodes into a temporary world and checks that floats are finite, that the timestep is positive, that a dynamic body has a positive mass, that IDs are unique and that each joint references two existing, distinct bodies. It then rebuilds the derived data (ID -> body index, joint indices) and only then replaces the world. A snapshot can be loaded into an empty world or replace an existing one. The snapshot is taken at the end of a step (`Step()`), and the simulation resumes identically after restoration (tested).

Derived joint references are **indices**, not pointers: they stay valid if the world is copied or moved.

## Tests

`AssetTests` (70 checks) and `AdapterTests` (24 checks) cover:

- round-trips of every primitive type, of both general types and of the physics scene;
- empty archives and collections;
- access by ID, entry listing, wrong type;
- V1 -> V2 migration and rejection of unknown versions (schema and format);
- truncation, an altered byte, a wrong magic, excessive sizes and counts, an offset that would overflow, a duplicate ID, type 0, an invalid reference (joint to a missing body);
- write failure (missing folder) and no leftover temporary file;
- no partially loaded state after each rejection.

Result: 100 % of the tests pass on Windows (MSVC) and Linux (GCC). Known defects: see "Limitations" below.

## Measurements

Command: `AssetDemo` (section 4). Each asset is an `Inventory` of 10 items; each value is the average of 5 runs, Release build.

| Assets | File size | Save | Load | Temp. memory (save) | Temp. memory (load) |
|---|---|---|---|---|---|
| 1,000 | 123,914 B | 1.8 ms | 4.9 ms | 219,804 B | 187,914 B |
| 100,000 | 12,588,914 B | 120 ms | 73 ms | 22,377,804 B | 18,988,914 B |

*Measured on a team PC (Windows, MSVC, Release). File sizes are identical on Linux/GCC: the format does not depend on the platform.*

- **Save**: serialization, building the final buffer and writing the file. **Load**: reading the file, full validation (including the CRC) and decoding every asset.
- **Method**: `std::chrono::steady_clock` around each phase. Temporary memory is **estimated by calculation** (buffer sizes: payloads kept by the `ArchiveWriter` + final buffer; on load, the whole file + the directory in memory), not measured by a system tool.
- **Limits**: the OS disk cache skews read times after the first run; the memory estimate ignores the small allocations of decoded `std::string` and `std::vector`. For the small volume, loading (4.9 ms) is slower than saving: with so little data, the fixed cost of opening the file on Windows (and possibly the antivirus) dominates. On 100,000 assets, this cost becomes negligible.

**Choices and costs**: the `ArchiveWriter` keeps each serialized payload, then copies them into the final buffer: memory during save is about twice the archive size. In exchange, the CRC and the offsets are computed on bytes that are already ready, and the file is written in one go. On load, the whole file is read into memory (simple and fast, but costly for a very large archive); payloads are then read through `std::span` without copying. The directory is indexed by an `std::unordered_map` (ID lookup in constant time on average).

## Limitations

- The archive is read entirely into memory (no partial loading from disk).
- The diagnostic for an invalid payload is generic ("invalid payload or unsupported schema version"): it does not say which field is wrong.
- Overlaps between payloads are not detected (each payload is only checked to be inside the data area).
- The CRC does not protect against deliberate tampering.
- The physics simulation is deliberately minimal (gravity only, no collisions): it is used to produce a state to restore. The physics structures are dedicated to the Asset Engine and are not yet connected to the engine's ECS.

---

## Sources

**Math library**: Intel Intrinsics Guide, nanobench.

**Asset Engine**:
- "Asset Engine" course sheet (GTech 3): `BinaryWriter` / `BinaryReader` primitives (section 6), range validation (section 8).
- CRC-32: standard algorithm described in the zlib manual (https://zlib.net/manual.html) and the PNG specification (https://www.w3.org/TR/png-3/).
- cppreference: `std::bit_cast`, `std::span`, `std::byte`.

## AI usage

**Math library**: Claude was used for explanations of SSE intrinsics, the benchmark protocol and the `run_bench.ps1` script (written by the AI), and interpretation hypotheses for the results.

**Asset Engine**: Claude was used for explanations of the course (binary encodings, archives, versioning) and diagrams of the architecture; for reviewing and debugging our `BinaryWriter` / `BinaryReader` (bounds checks, 64-bit reads, booleans); and, the tests, the demo and a first version of this README. We then reviewed this code, ran it on our machines, and checked its behaviour with the automated tests.