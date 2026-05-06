# Copilot Instructions — Apeirogon (PhotoTools)

## Mandatory Code Review Workflow

> **Every non-trivial code change MUST be reviewed by the `Critical Review` agent before it is considered done.**
>
> After implementing a fix or feature, invoke the **Critical Review** agent to run its three-phase pipeline (assumption audit → claim verification → risk report). Only proceed to commit once all CRITICAL and HIGH findings are resolved.
>
> Trivial changes (typo fixes, comment updates, single-line renaming) are exempt.

## Project Overview

**Apeirogon** is a C++20 library for fast, robust 3D Boolean operations (union, intersection, difference, symmetric difference) on polyhedra. The core innovation is **plane-based implicit coordinates**: instead of storing vertex positions as floating-point XYZ, vertices are represented as the intersection of three planes. This allows exact geometric predicates via interval arithmetic and error-tracking number types, avoiding floating-point inconsistencies.

## Build System

**Prerequisites:** CMake ≥ 3.21, Ninja, vcpkg (bundled as a submodule).

```bash
# Configure (release, Ninja, vcpkg)
cmake --preset ninja-release-vcpkg

# Build
cmake --build --preset ninja-release-vcpkg

# Run all registered tests
cd builds/ninja-release-vcpkg && ctest

# Run a single named test
cd builds/ninja-release-vcpkg && ctest -R ExactArithmetic
```

The Boost.Test executable is `PB2Test`. To run a specific Boost test suite directly:
```bash
./builds/ninja-release-vcpkg/bin/PB2Test --run_test=PlaneBasedGeometryTests/ExactArithmetic \
  --logger=HRF,all --color_output=false --report_format=HRF --show_progress=no
```

Test input data and output directories are compile-time constants:
- `PATH_TO_INPUTDATA` → `FunctionTests/InputData/`
- `PATH_TO_OUTPUT` → `FunctionTests/Output/`

### Sanitizer Presets

Four sanitizer presets are available (all use `RelWithDebInfo`, mimalloc disabled):

| Preset | Sanitizer | Use for |
|---|---|---|
| `ninja-asan` | AddressSanitizer | heap/stack/global buffer overflows, use-after-free |
| `ninja-ubsan` | UndefinedBehaviorSanitizer | signed overflow, bad casts, null deref, OOB |
| `ninja-asan-ubsan` | ASan + UBSan combined | best daily-driver; catches both classes of bugs |
| `ninja-tsan` | ThreadSanitizer | data races in TBB-parallel code — **mutually exclusive with ASan/LSan** |

```bash
cmake --preset ninja-asan-ubsan
cmake --build --preset ninja-asan-ubsan
cd builds/ninja-asan-ubsan && ctest
```

Sanitizers print a symbolized report to stderr automatically (no debugger needed). To attach lldb on macOS and catch the abort:
```bash
ASAN_OPTIONS=abort_on_error=1 UBSAN_OPTIONS=print_stacktrace=1 \
  lldb -- ./builds/ninja-asan-ubsan/bin/PB2Test --run_test=MySuite/MyTest
```

Useful runtime options:
- `ASAN_OPTIONS=detect_leaks=1` — enable LSan on macOS (off by default)
- `UBSAN_OPTIONS=print_stacktrace=1` — add stack trace to UBSan reports
- `TSAN_OPTIONS=halt_on_error=1` — stop on first race

## Module Structure

| Directory | CMake Target | Output | Role |
|---|---|---|---|
| `Infrastructure/` | `PB2Infra` | static lib | Logging (spdlog/fmt), error management, utilities, multithreading settings |
| `Mathematics/` | `PB2Maths` | static lib | Exact arithmetic number types: `DoubleDynamicError`, `IntervalArithmetic`, `FloatRingExtension`; predicate engine |
| `PlaneBasedGeometry/` | `PB2Geo` | shared lib | Core geometry objects: `Plane`, `Point`, `Polygon`, `Polyhedron`, `PolyhedronTopologyGraph` |
| `PlaneBasedOperators/` | `PB2Operators` | shared lib | Boolean operator pipeline, spatial localizers (RTree/Embree), split operations, tag-and-sort |
| `PlaneBasedIO/` | `PB2IO` | shared lib | Import/export (Assimp, Poco ZIP/JSON/XML) |
| `PlaneBasedTests/` | `PB2Test` | executable | Boost.Test suite |
| `PBCLI/` | CLI executable | executable | Command-line interface |
| `PerformanceTests/` | — | executable | Performance benchmarks |

External libraries (`assimp`, `libigl`) are included as subdirectories. vcpkg manages all other dependencies (see `vcpkg.json`).

## Core Architecture

### Plane-Based Coordinate System
- A `PlaneBasedGeoObjects::Point` is defined as the intersection of **three planes** (`GetP1/P2/P3()`).
- Two concrete types: `PointBy3Planes` (pure plane representation) and `PointCartesian` (Cartesian with optional plane backing).
- Similarly, `PlaneBasedGeoObjects::Plane` has two subtypes: `PlaneCartesian` and `PlaneThrough3Points`.
- **Never use `auto`** with Eigen types (see comment in `CartesianGeometry.h`). Pass Eigen objects by `const&`, never by value.

### Exact Arithmetic
`Mathematics/SharedInterfaces/` provides three number types used for predicate evaluation:
- `DoubleDynamicError` — double with dynamic error tracking
- `IntervalArithmetic` — interval-based bounds
- `FloatRingExtension` — multi-precision float ring

`Plane` and `Point` expose `GetStdEquationTpl<NT>()` / `GetCartesianRepresentationTpl<NT>()` template accessors to retrieve equations in any of these types.

### Factory Pattern for Geometric Objects
- `GeoObjectFactory` (abstract) is the single entry point for creating `Plane` and `Point` objects. Factories de-duplicate objects using flyweight patterns.
- `FactoryOfOperators::CreatePBBooleanOper()` creates the Boolean operator implementation.

### Boolean Operator Pipeline
The pipeline in `PlaneBasedOperators/` follows these steps:
1. **Spatial localization** — find pairs of potentially-intersecting polygons via RTree (`RTreeSpatialPolyhedronsIntersectionsLocalizer`) or Embree LBVH.
2. **Polygon splitting** — `SplitPolygonByPlaneOpe`, `SplitPolygonByPolylineOpe`, `SplitCoplanarPolygonOpe`.
3. **Tag and sort** — `TagAndSortPolygonsOpe` classifies fragments as inside/outside using a graph diffusion algorithm.
4. **Topology update** — `PolyhedronTopologyUpdater` rebuilds neighbor links.

### Topology Graph
`PolyhedronTopologyGraph` (in `PlaneBasedGeometry/`) wraps a `boost::adjacency_list` mapping polygon pointers to graph vertices. It is lazily computed and cached on `Polyhedron`; the cache is invalidated when the polygon set changes (`InvalidateTopologyGraph()`).

Two feature flags control topology graph behavior at runtime (environment variables):
- `PB_TAG_GRAPH_DIFFUSION_ENABLE` — enables topology-graph-based diffusion in `TagAndSortPolygonsOpe`.
- `PB_TOPOLOGY_GRAPH_VALIDATE` — enables validation of the topology graph (set to `1`, `Y`, `y`, `T`, or `t`).

### Polyhedron
`Polyhedron` stores polygons as a `std::map<shared_ptr<Polygon>, char>` (char = orientation: `+1` for outward normal, `-1` for inward). Polygon pointers are compared by raw address, so polygon identity is pointer-based.

## Key Conventions

### Naming
- Input parameters: `i` prefix (e.g., `iOperand`, `iPlaneEquation`).
- Output parameters: `o` prefix (e.g., `oIsValid`).
- In/out parameters: `io` prefix.
- Member data: `m_` prefix.
- Namespaces are aliased at file scope: `namespace PBGO = PlaneBasedGeoObjects;`, `namespace PBOP = PlaneBasedOperators;`, `namespace CG = CartesianGeometry;`.

### DLL Export Macros
Each module has its own export macro (`ExportedByPlaneBasedGeometry`, `ExportedByPlaneBasedInfra`, `ExportedByPlaneBasedOperators`). These expand to `__declspec(dllexport)` on Windows and are empty elsewhere. Apply them to all public class declarations in shared libraries.

### Non-Copyable Classes
Use `DISALLOW_COPYCTOR_ASSIGN_AND_MOVE(MyType)` macro (from `RuleOf5CLASSMACROS.h`) to explicitly delete copy/move constructors and assignment operators for non-copyable types.

### Logging
Use the free functions from `Logger.h` (backed by spdlog/fmt):
```cpp
LogError({}, "message with {} arg", value);   // logs + attach source location
LogWarning({}, "...");
LogInfo("...");
LogDebug("...");
```
The first argument `{}` constructs a `LocMsg` capturing `std::source_location::current()` automatically. Do **not** use `spdlog` directly in business code.

### Error Handling
Return `ErrorKinds` enum values (`PB_OK`, `KO_*`) from operations; throw `std::invalid_argument` for null-pointer guards using `ThrowForNullPointer(ptr)` from `ErrorManagement.h`.

### No Tolerances
All geometric predicates and arithmetic are **exact** — never use epsilon/tolerance comparisons in geometric logic. The cascade `DoubleDynamicError` → `IntervalArithmetic` → `FloatRingExtension` guarantees exact sign decisions without any floating-point fuzz. The **only** legitimate use of tolerances is during primitive creation (e.g., mesh import, plane fitting from noisy input data); once a `Plane` or `Point` object exists, all subsequent decisions must be exact.

### Code Style
Formatting is enforced via `.clang-format` (LLVM base, 2-space indent, no column limit, braces on new lines for classes/functions/control flow). clang-tidy is configured in `.clang-tidy` (bugprone, modernize, performance, readability checks enabled).

### Tests
- Tests use **Boost.Test** (`boost::test`) via the `PB2Test` executable.
- Test files that should not currently build are suffixed `.cpp_nobuild` (e.g., `TestMASSimuPB2_SimpleCases.cpp_nobuild`). Do not rename or add to CMakeLists without verifying they compile.

## Dependencies (vcpkg)
Key packages: `eigen3`, `boost` (test, flyweight, graph, interval, math, program-options, unordered), `tbb` (OneTBB), `spdlog`, `fmt`, `poco` (XML/JSON/ZIP), `gtl` (parallel hash maps), `embree`, `nlohmann-json`, `xsimd`, `magic-enum`, `indicators`, `benchmark`.
