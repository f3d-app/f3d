# AGENTS.md — Apeirogon

## Mandatory Code Review Workflow

> **Every non-trivial code change MUST be reviewed by the `Critical Review` agent before it is considered done.**
>
> This is not optional. After implementing a fix or feature, invoke the **Critical Review** agent to run its three-phase pipeline (assumption audit → claim verification → risk report). Only proceed to commit once all CRITICAL and HIGH findings are resolved.
>
> Trivial changes (typo fixes, comment updates, single-line renaming) are exempt.

## What This Project Is

C++20 library for exact 3D Boolean operations (union, intersection, difference) on polyhedra. Vertices are represented as the intersection of three planes (not XYZ floats), enabling exact geometric predicates via interval/error-tracking arithmetic.

## Build & Test

```bash
cmake --preset ninja-release-vcpkg
cmake --build --preset ninja-release-vcpkg
cd builds/ninja-release-vcpkg && ctest                    # all tests
ctest -R ExactArithmetic                                   # single test
./bin/PB2Test --run_test=PlaneBasedGeometryTests/ExactArithmetic  # Boost.Test direct
```

Test data paths are compile-time macros: `PATH_TO_INPUTDATA` → `FunctionTests/InputData/`, `PATH_TO_OUTPUT` → `FunctionTests/Output/`.

### Sanitizer Presets

| Preset | Sanitizer | Notes |
|---|---|---|
| `ninja-asan` | AddressSanitizer | buffer overflows, use-after-free |
| `ninja-ubsan` | UndefinedBehaviorSanitizer | signed overflow, bad casts, OOB |
| `ninja-asan-ubsan` | ASan + UBSan | best general-purpose choice |
| `ninja-tsan` | ThreadSanitizer | data races — **incompatible with ASan/LSan** |

All sanitizer presets use `RelWithDebInfo` and disable mimalloc. Reports are printed to stderr automatically; no debugger required. Key runtime options: `ASAN_OPTIONS=abort_on_error=1`, `UBSAN_OPTIONS=print_stacktrace=1`, `TSAN_OPTIONS=halt_on_error=1`.

## Module Map

| Directory | Target | Type | Purpose |
|---|---|---|---|
| `Infrastructure/` | `PB2Infra` | static lib | Logging, errors, utilities, macros |
| `Mathematics/` | `PB2Maths` | static lib | `DoubleDynamicError`, `IntervalArithmetic`, `FloatRingExtension` |
| `PlaneBasedGeometry/` | `PB2Geo` | shared lib | `Plane`, `Point`, `Polygon`, `Polyhedron`, `PolyhedronTopologyGraph` |
| `PlaneBasedOperators/` | `PB2Operators` | shared lib | Boolean pipeline, spatial localizers, split/tag/sort |
| `PlaneBasedIO/` | `PB2IO` | shared lib | Import/export (Assimp, Poco) |
| `PlaneBasedTests/` | `PB2Test` | executable | Boost.Test suite |

Public headers live in `<Module>/SharedInterfaces/`, internal headers in `<Module>/LocalInterfaces/`.

## Critical Conventions

**Naming** — Parameters: `i` (input), `o` (output), `io` (in/out). Members: `m_` prefix. Namespace aliases at file scope:
```cpp
namespace PBGO = PlaneBasedGeoObjects;
namespace PBOP = PlaneBasedOperators;
namespace CG = CartesianGeometry;
```

**Eigen types** — Never use `auto` with Eigen expressions (aliasing bugs). Pass by `const&`, never by value. See comment in `CartesianGeometry.h`.

**DLL export** — Each shared-lib module has its own macro (`ExportedByPlaneBasedGeometry`, `ExportedByPlaneBasedOperators`, etc.). Apply to all public class declarations. These are no-ops on non-Windows.

**Non-copyable** — Use `DISALLOW_COPYCTOR_ASSIGN_AND_MOVE(MyType)` from `RuleOf5CLASSMACROS.h`.

**Logging** — Use `LogError({}, "msg {}", val)`, `LogWarning({}, ...)`, `LogInfo(...)`, `LogDebug(...)` from `Logger.h`. The `{}` first arg auto-captures `std::source_location`. Never call spdlog directly.

**Errors** — Return `ErrorKinds` enum (`PB_OK`, `KO_*`). Guard null pointers with `ThrowForNullPointer(ptr)` from `ErrorManagement.h`.

**No tolerances** — All geometric predicates and arithmetic are **exact**. Never use epsilon/tolerance comparisons in geometric logic. The cascade `DoubleDynamicError` → `IntervalArithmetic` → `FloatRingExtension` guarantees exact sign decisions. The **only** legitimate use of tolerances is during primitive creation (mesh import, plane fitting from noisy input); once a `Plane` or `Point` object exists, all decisions must be exact.

## Key Architectural Patterns

**Plane-based points** — `PBGO::Point` = intersection of 3 planes (`GetP1()/P2()/P3()`). Two subtypes: `PointBy3Planes`, `PointCartesian`. Template accessors `GetStdEquationTpl<NT>()` / `GetCartesianRepresentationTpl<NT>()` allow evaluation in any exact number type.

**Factory + Flyweight** — Always create `Plane`/`Point` via `GeoObjectFactory` (deduplicates objects). Create Boolean operators via `FactoryOfOperators::CreatePBBooleanOper(iPairFinder)`.

**Polyhedron** — Stores `boost::unordered_node_map<shared_ptr<Polygon>, char>` where `char` = orientation (`+1` outward, `-1` inward). Polygon identity is pointer-based (hash/compare on raw address). Topology graph (`PolyhedronTopologyGraph`, boost::adjacency_list) is lazily built and cached; invalidated on polygon-set mutation.

**Boolean pipeline** — 1) Spatial localization (RTree or Embree LBVH) → 2) Polygon splitting (`SplitPolygonByPlaneOpe`, etc.) → 3) Tag & sort (`TagAndSortPolygonsOpe`, graph diffusion) → 4) Topology update.

## Tests

- Boost.Test with nested suites: `BOOST_AUTO_TEST_SUITE(PlaneBasedGeometryTests) / BOOST_AUTO_TEST_SUITE(ExactArithmetic)`.
- Helper class `TstPolyhedronRefController` validates expected polygon counts; `TestUtilities::RunAndCheckBoolean(...)` is the standard assertion wrapper.
- Files suffixed `.cpp_nobuild` are intentionally excluded from the build — do not add them to CMakeLists.
- Shapes for test input: `PBGO::Shapes::MakeFromAABB(box)`, `MakeTetrahedron(...)`, `MakeCone(...)`.

## Geometric Predicate Engine

### Overview

All exact geometric predicates live in `PlaneBasedGeometry/`. Two complementary engines are available:

| Engine | Header | Purpose |
|---|---|---|
| `PlaneBasedMathematics::RobustEvalSign<CacheSize, Functor>(doubles...)` | `Mathematics/SharedInterfaces/PredicatesEngine.h` | Low-level: takes raw `double` scalars; includes `ClearFloatingPointStatus()` + `OverflowOrUnderflow()` guards around every pass |
| `GeoPredicates::Evaluators::EvalSign<Functor>(wrappers...)` | `PlaneBasedGeometry/SharedInterfaces/GeoEvaluators.h` | High-level: takes `AsPlane`/`AsPoint`/`AsCartesian3D` wrappers; fetches equations via `GetStdEquationTpl<NT>()` / `GetCartesianRepresentationTpl<NT>()` |

Both cascade through arithmetic types in order: `DoubleDynamicError` → `IntervalArithmetic` → `FloatRingExtension`, returning as soon as a definite `Sign_t` (`POSITIVE`, `NEGATIVE`, `ZERO`) is obtained. `SignArithmetic` is intentionally excluded from GeoEvaluators (approximated coords may produce an incorrect early exit).

### GeoEvaluators API

**Three engine entry points:**

```cpp
// 1. EvalSign<Functor> — stateless template-template functor
//    CacheSize > 0 adds an LRU cache on the FRE pass (keyed on FRE-fetched arrays).
Sign_t EvalSign<Functor, CacheSize = 0>(Inputs... inputs);

// 2. EvalSignWith — runtime callable with templated operator()<NT>
//    Good for index-capturing lambdas.
Sign_t EvalSignWith(callable, Inputs... inputs);

// 3. EvalSignWithDirect — callable returns Sign_t directly (not NT)
//    Use for predicates with cross-pass mutable state (e.g., ioLSigns row-sign cache).
Sign_t EvalSignWithDirect(callable, Inputs... inputs);
```

**Input wrappers:**

```cpp
AsPlane{plane}         // calls plane.GetStdEquationTpl<NT>()
AsPoint{point}         // calls point.GetCartesianRepresentationTpl<NT>()
AsCartesian3D{xyz}     // promotes double[3] → array<NT,4> with W=1
```

**Built-in functors** (all in `GeoPredicates::Evaluators`):

| Functor | Inputs | Computes |
|---|---|---|
| `EvalPlaneAtHomogeneousPoint` | `AsPlane, AsPoint` | `a·X+b·Y+c·Z+d·W`; nullopt if W=0 |
| `Eval3PlaneNormalsDeterminant` | 3× `AsPlane` | 3×3 det of normals — non-zero iff point exists |
| `EvalTripleProductPlanePlanePtPt` | 2× `AsPlane`, 2× `AsPoint` | det3×3(plane0_normals, edge(pt0→pt1), plane1_normals) |
| `EvalTripleProductPlanePtPtPt` | `AsPlane`, 3× `AsPoint` | det3×3(edge01, edge12, planeSupport_normals) |

### Adding a New Predicate

1. **Define a functor** as `template<typename NT> struct MyFunctor` whose `operator()` takes `const std::array<NT,4>&` per input and returns `NT` or `std::optional<NT>` (return `nullopt` to signal "escalate to next arithmetic"):

```cpp
template<typename NT>
struct MyFunctor
{
  [[nodiscard]] NT operator()(const std::array<NT, 4>& iPlane,
                               const std::array<NT, 4>& iPt) const
  {
    return iPlane[0] * iPt[0] + iPlane[1] * iPt[1] + iPlane[2] * iPt[2];
  }
};
```

2. **Call the engine:**

```cpp
Sign_t sign = GeoPredicates::Evaluators::EvalSign<MyFunctor>(
    GeoPredicates::Evaluators::AsPlane{iPlane},
    GeoPredicates::Evaluators::AsPoint{iPoint});
```

3. For runtime-parameterized predicates (e.g., varying column indices), use `EvalSignWith` with a generic lambda:

```cpp
Sign_t sign = GeoPredicates::Evaluators::EvalSignWith(
    []<typename NT>(const std::array<NT,4>& p1, const std::array<NT,4>& p2)
    { return p1[col_i] * p2[col_j] - p1[col_j] * p2[col_i]; },
    GeoPredicates::Evaluators::AsPlane{iPlane1},
    GeoPredicates::Evaluators::AsPlane{iPlane2});
```

### Limitation vs RobustEvalSign

Both engines now apply identical FP-status guards (`ClearFloatingPointStatus()` before, `OverflowOrUnderflow()` after) around the DDE and Interval passes. The remaining difference is that `RobustEvalSign` uses the specialized `From2x2Determinant` constructor internally, which gives **tighter DDE error bounds** via error-free transformations. For predicates on near-degenerate or tiny-coefficient inputs (e.g., planes with normals ~1e-9), this extra DDE tightness can resolve the sign without escalating to FRE. `SignOf3PlanesDeterminant` keeps a Cartesian fast-path through `RobustEvalSign<1024U, Eval3x3DeterminantT>` for this reason.

### Public Predicate API

All public predicates are declared in `PlaneBasedGeometry/SharedInterfaces/BasicGeometricPredicates.h` (namespace `GeoPredicates`) and implemented in `BasicGeometricPredicates.cpp`:

| Function | Returns | Description |
|---|---|---|
| `IsPlaneValid(Plane)` | `bool` | Normal vector is nonzero |
| `IsPointValid(Point)` | `bool` | 3-plane intersection is non-degenerate (det≠0) |
| `ArePlanesCoincident(P1, P2)` | `bool` | All 2×2 minors of [P1; P2] are zero |
| `ArePlanesParallel(P1, P2)` | `bool` | Normal cross-product is zero |
| `CoincidentOrientation(P1, P2)` | `bool` | All products `Pi·Qi` are non-negative |
| `ArePointsEqual(A, B)` | `bool` | All plane combinations evaluate to zero |
| `IsPointInFrontOfPlane(Plane, Point)` | `TriBool` | `True`=positive side, `False`=negative, `Neutral`=on plane |
| `IsPointOnLine(Line, Point)` | `bool` | Point lies on both planes defining the line |
| `SignOf3PlanesDeterminant(P1,P2,P3)` | `int` | ±1 or 0; indicates if 3 planes form a valid point |
| `SignOfTripleProduct(...)` | `int` | ±1 or 0; orientation tests for splitting |

`TriBool` values: `TriBool::True` (+1), `TriBool::False` (-1), `TriBool::Neutral` (0), `TriBool::Unknown`.

## Formatting

`.clang-format`: LLVM base, 2-space indent, no column limit, braces on new line for classes/functions/control-flow. `.clang-tidy` enables bugprone, modernize, performance, readability checks.

## Runtime Feature Flags (env vars)

- `PB_TAG_GRAPH_DIFFUSION_ENABLE` — enable topology-graph diffusion in tag-and-sort.
- `PB_TOPOLOGY_GRAPH_VALIDATE` — validate topology graph (`1`/`Y`/`y`/`T`/`t`).

