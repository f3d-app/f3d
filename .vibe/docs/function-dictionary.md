# Apeirogon Function Dictionary

> AI-readable quick-reference. Prefer reading this before opening source files.
> Last updated: refreshed after the PB2Test / PB2SlowTest test-suite split.

---

## Module Structure Tree

```
Apeirogon/
├── Infrastructure/          (PB2Infra)    — cross-cutting: logging, errors, utilities, flyweights
│   └── SharedInterfaces/
│       ├── Logger.h                       — spdlog wrappers + RAISE_ERROR macro
│       ├── ErrorManagement.h              — ThrowForNullPointer<T>, ErrorKinds enum
│       ├── RuleOf5CLASSMACROS.h           — DISALLOW_COPYCTOR_ASSIGN_AND_MOVE macro
│       ├── BaseUtils.h                    — PBUtils: ReleasePtr, DeletePtr, ToVectorOfPtr
│       ├── TriBool.h                      — enum class TriBool {False=-1,Neutral=0,True=1,Unknown=2}
│       ├── ConstantsDefinitions.h         — DefaultSettings namespace (tolerances, grid params)
│       ├── PerfoTracker.h                 — PerfoTracker, ScopedPerfoTimer profiling classes
│       ├── MultiThreadingSettings.h       — IsMonothreadForced() via DISABLE_MT env var
│       ├── unordered_flyweight.h          — unordered_flyweight<T> alias (Boost flyweight + node_set)
│       ├── concurrent_flyweight.h         — concurrent_flyweight<T> alias (lock-free + GC thread)
│       ├── Attribute.h                    — PBInfra::AttributeGroup, Color attachment
│       ├── Color.h                        — PBInfra::Color
│       ├── Chronos.h                      — wall-clock timer helper
│       ├── MemUsage.h                     — resident set size helpers
│       ├── PolyhedronStatsResult.h        — Apeirogon::Infra::Stats_t aggregate struct
│       ├── PlottedReports.h               — chart/reporting helpers
│       ├── STLExtensions.h                — small STL helpers
│       └── Version.h                      — build version constants
│
├── Mathematics/             (PB2Maths)    — exact & filtered arithmetic for geometric predicates
│   └── SharedInterfaces/
│       ├── DoubleDynamicError.h           — DoubleDynamicError (semi-static filter)
│       ├── IntervalArithmetic.h           — IntervalArithmetic typedef (Boost interval)
│       ├── FloatRingExtension.h           — FloatRingExtension (exact rational via LargeInteger)
│       ├── LargeInteger.h                 — arbitrary-precision integer backing FloatRingExtension
│       ├── TypeDefinitions.h              — Sign_t enum, MakeVar<NT> factory template
│       ├── PredicatesEngine.h             — generic sign predicate dispatcher
│       ├── PredicatesUtilities.h          — SignOf<NT>, ToDouble<NT>, LowerBound/UpperBound
│       ├── SignArithmetic.h               — sign propagation helpers
│       ├── BasicMathsPredicates.h         — point-on-plane, orientation predicates (double)
│       ├── Evaluators.h                   — evaluator adaptors for NT types
│       ├── FloatingPointStatus.h          — FP exception/rounding state helpers
│       ├── FloatingPointPragmas.h         — DISABLE_OPTIMIZATIONS / ENABLE_OPTIMIZATIONS macros
│       ├── FloatingPointsRounding.h       — FloatingPointsRounding RAII guard (interval mode)
│       ├── MathsPredicatesInfra.h         — infrastructure for predicate layers
│       ├── PBEAEnums.h                    — exact-arithmetic enum helpers
│       └── ExportedByPlaneBasedMathematics.h
│
├── PlaneBasedGeometry/      (PB2Geo)      — core plane-based geometry objects
│   └── SharedInterfaces/
│       ├── Plane.h                        — abstract Plane base class + factory statics
│       ├── PlaneCartesian.h               — PlaneCartesian (double[4] coefficients)
│       ├── PlaneThrough3Points.h          — PlaneThrough3Points (lazy exact equations)
│       ├── Point.h                        — abstract Point base class + factory statics
│       ├── PointBy3Planes.h               — PointBy3Planes (intersection of 3 planes)
│       ├── PointCartesian.h               — PointCartesian (xyz + 3 canonical planes)
│       ├── CartesianPointFlyweight.h      — FlyWeightPt_t alias for interned Point3D
│       ├── CartesianGeometry.h            — CartesianGeometry namespace: Point3D/Vector3D (Eigen)
│       ├── Edge.h                         — Edge (begin/end Point, Plane, neighbor Polygon)
│       ├── PolygonLoop.h                  — PolygonLoop (small_vector-based closed chain)
│       ├── Polygon.h                      — Polygon (supporting Plane + loops)
│       ├── Polyhedron.h                   — Polyhedron (boost::unordered_node_map<Polygon,char>)
│       ├── PolyhedronTopologyGraph.h      — PolyhedronTopologyGraph (Boost.Graph adjacency_list)
│       ├── GeoObjectFactory.h             — GeoObjectFactory abstract interface
│       ├── BasicGeometricPredicates.h     — low-level geometric predicates
│       ├── Line.h                         — Line (two points)
│       ├── Polyline.h                     — Polyline (ordered point sequence)
│       ├── PolygonAttribute.h             — per-polygon attribute helpers
│       ├── Shapes.h                       — primitive shape builders (box, sphere …)
│       ├── Box3D.h, OBox3D.h              — axis-aligned and oriented bounding boxes
│       ├── RefCountDeVirtualised.h        — intrusive ref-count base
│       └── ExportedPlaneBasedGeometry.h
│   └── src/
│       ├── Plane.cpp, PlaneCartesian.cpp, PlaneThrough3Points.cpp
│       ├── Point.cpp, PointBy3Planes.cpp, PointCartesian.cpp
│       ├── Edge.cpp, PolygonLoop.cpp, Polygon.cpp, Polyhedron.cpp
│       ├── PolyhedronTopologyGraph.cpp, BasicGeometricPredicates.cpp
│       └── CartesianGeometry.cpp, Polyline.cpp, Shapes.cpp
│
├── PlaneBasedOperators/     (PB2Operators) — Boolean pipeline, split ops, spatial localisers
│   ├── SharedInterfaces/
│   │   ├── BooleanOperator.h              — abstract BooleanOperator interface
│   │   ├── BooleanCommonTypes.h           — OrientedPolygon_t, TwoOrientedPolygons_t, PolygonToFragmentsJournal_t
│   │   ├── SuperGraphTagLocalizer.h       — SuperGraphTagLocalizer (persistent super-graph)
│   │   ├── FindPairsOfPolygonsToIntersect.h — abstract pair-finder interface
│   │   ├── RTreeSpatialPolyhedronsIntersectionsLocalizer.h — RTree-based pair finder
│   │   ├── EmbreeLBVHSpatialPolyhedronsIntersectionsLocalizer.h — Embree LBVH pair finder
│   │   ├── SpatialPolyhedronsIntersectionsLocalizer.h — generic spatial localiser wrapper
│   │   ├── SplitPolygonByPlaneOpe.h       — split one polygon by a plane
│   │   ├── SplitPolygonByPolylineOpe.h    — split one polygon by a polyline
│   │   ├── SplitCoplanarPolygonOpe.h      — split coplanar polygons
│   │   ├── SplitCoplanarPolygonsOpe.h     — batch coplanar split operator
│   │   ├── IntersectionPredicates.h       — polygon-polygon intersection predicates
│   │   ├── AdvancedPredicates.h           — higher-level predicates
│   │   ├── AdvSortingPredicates.h         — sorting predicates for tag/sort
│   │   ├── AdvClassificationPredicates.h  — polygon classification predicates
│   │   ├── PolygonvsPlanePosition.h       — polygon half-space classification
│   │   ├── GeometricProperties.h          — area, volume, mass properties of polyhedra
│   │   ├── PolyhedronStats.h              — polygon/polyhedron statistics
│   │   ├── PolyhedronExporter.h           — export polyhedron to mesh formats
│   │   ├── PolyhedronTopologyUpdater.h    — incremental topology graph updater
│   │   ├── TopoHealer.h                   — topology healing utilities
│   │   ├── EquivalentObjects.h            — equivalence helpers for planes/points
│   │   ├── RTree.h                        — generic R-Tree template
│   │   ├── BinarySpatialTree.h/.Node.h    — BSP tree
│   │   ├── PolylineSegmentsChaining.h     — polyline segment chaining utility
│   │   ├── TriMeshImporter.h              — abstract mesh importer interface
│   │   ├── InfinityPointsFactory.h        — factory for "infinity" sentinel points
│   │   ├── ManufacturingAttribute.h       — manufacturing face attribute
│   │   ├── DebugUtilities.h               — debug/visualization helpers
│   │   ├── FactoryOfOperators.h           — FactoryOfOperators::CreatePBBooleanOper, CreateTriMeshImporter
│   │   └── static_sort.h                  — compile-time sorting network
│   └── LocalInterfaces/
│       ├── BooleanOperatorImpl.h          — BooleanOperatorImpl (concrete Boolean pipeline)
│       ├── TagAndSortPolygonsOpe.h        — TagAndSortPolygonsOpe (tag diffusion + sort)
│       ├── ParallelTagDiffusion.h         — parallel tag diffusion helper
│       ├── PolygonPolygonLineIntersectionsOpe.h — polygon-polygon line intersections
│       ├── PolygonsMerger.h               — merge coplanar polygons
│       ├── PatchesOfPolyhedron.h          — patch connectivity helpers
│       ├── PointOnEdgeFactory.h           — deduplicated point-on-edge factory
│       ├── PlaneUpdater.h                 — batch plane replacement helper
│       ├── TopoTools.h                    — low-level topology tools
│       ├── IntersectionPoint.h            — IntersectionPoint (point on two intersecting polygons)
│       ├── UpdateNeighboorLink.h          — neighbour link updater after split
│       └── TriMeshImporterImpl.h          — TriMesh → Polyhedron importer implementation
│
├── PlaneBasedIO/            (PB2IO)       — mesh import/export via Assimp
│   └── SharedInterfaces/
│       └── PBIO.h                         — PlaneBasedIO::ReadMesh, WriteMesh (Assimp ↔ Eigen)
│
├── PlaneBasedTests/         (PB2Test)     — main Boost.Test regression suite
│   ├── LocalInterfaces/
│   │   ├── TestUtilities.h                — TestUtilities namespace: checkers, mesh-import/boolean helpers
│   │   ├── SpatialLocalizerComparisonFixture.h — shared cube/world-box fixture reused by correctness + slow tests
│   │   ├── TstPolyhedronRefController.h   — reference polyhedron comparison controller
│   │   ├── TstMathsTools.h                — maths test helpers
│   │   └── TestParameters.h               — test parameter constants + logger observer
│   └── src/
│       ├── main.cpp                       — BOOST_TEST_MODULE PB2Test + logger-backed Boost formatter
│       ├── TestTriMeshImporter.cpp        — retained importer regression subset (Tetra, Pad12288T, ToreCreux...)
│       ├── TestTriMeshImportAndMerge.cpp  — retained merge/import regression subset (CubeTroue, MengerSponge_0..2)
│       ├── TestSpatialLocalizerComparison.cpp — correctness-only BSP vs RTree checks
│       └── ... module suites: MathematicsTests / PlaneBasedGeometryTests / PlaneBasedOperatorsTests / PlaneBasedIOTests / InfrastructureTests
│
├── PerformanceTests/        (PerformanceTests + PB2SlowTest) — CLI perf harness + slow/benchmark Boost suites
│   └── src/
│       ├── main.cpp, TestsPerformance.h/.cpp — command-line iteration benchmarks + localizer selection
│       ├── SlowTestMain.cpp               — BOOST_TEST_MODULE PB2SlowTest + logger-backed Boost formatter
│       ├── TestBooleansPerformance.cpp    — long-running import+boolean integration suites
│       ├── TestTriMeshImporter.cpp        — heavy importer cases moved out of PB2Test
│       ├── TestTriMeshImportAndMerge.cpp  — heavy merge/import cases moved out of PB2Test
│       └── TestSpatialLocalizerPerformance.cpp — benchmark-style BSP vs RTree comparison
│
└── PBCLI/                   (CLI)         — command-line driver
    └── src/main.cpp                       — entry point: loads mesh, runs Boolean ops via Boost.ProgramOptions
```

---

## Module: `Infrastructure/` — PB2Infra

Cross-cutting utilities shared by all other modules.

### `Infrastructure/SharedInterfaces/Logger.h`
Thin wrappers over **spdlog** (file + console sinks). Pure header; no class required.

- `LocMsg` — captures `std::source_location` at construction; `format() -> std::string` produces `"File: … Line: … | Function: …"`.
- `LogError(LocMsg, fmt, args...)` — formats `[E] msg | location` and forwards to `spdlog::error`.
- `LogWarning(LocMsg, fmt, args...)` — `[w]` prefix, `spdlog::warn`.
- `LogInfo(fmt, args...)` — `spdlog::info` (no location).
- `LogDebug(fmt, args...)` — `spdlog::debug`.
- `InitializeLogger(logFilePath, consoleOutput=true) -> void` — creates multi-sink logger; must be called once at startup before any log call.
- `RAISE_ERROR(ExcType, Msg)` macro — logs error then throws `ExcType(Msg)`.
- `FMT_RAISE_ERROR(ExcType, Msg, ...)` macro — formatted variant.

### `Infrastructure/SharedInterfaces/ErrorManagement.h`
Depends on `Logger.h`.

- `ThrowForNullPointer<Ptr>(ptr, location)` — template constrained to raw/shared/unique pointer; logs + throws `std::invalid_argument` if null. **Gotcha**: constraint checks `shared_ptr<element_type>` via SFINAE — use `ThrowForNullPointer(myShrd)` not `ThrowForNullPointer<shared_ptr<T>>(myShrd)`.
- `enum ErrorKinds { PB_OK=0, KO_CRITICAL_INIT_ERROR, KO_INVALID_SIZE, KO_INVALID_INPUT_TOLERANCE, KO_INVALID_POLYHEDRON, KO_INVALID_POLYGON, KO_UNLINKABLE_EDGES, KO_NULL_POINTER_ERROR, KO_INTERNAL_ERROR }` — return codes used throughout the pipeline.

### `Infrastructure/SharedInterfaces/RuleOf5CLASSMACROS.h`
- `DISALLOW_COPYCTOR_ASSIGN_AND_MOVE(MyType)` macro — deletes copy ctor, move ctor, copy-assign, move-assign. Used on non-copyable operator and factory classes.

### `Infrastructure/SharedInterfaces/BaseUtils.h`
`namespace PBUtils`

- `ReleasePtr<NT>(ptr)` — calls `ptr->Release()` then sets to nullptr; for COM-style intrusive ref counts.
- `DeletePtr<NT>(ptr)` — raw `delete` then nullptr.
- `ToVectorOfPtr<Container>(in) -> vector<element_type*>` — extracts raw pointers from a container of `shared_ptr`.
- `ToVectorOfSharedPtr<T>(in) -> vector<shared_ptr<T>>` — rebuilds shared_ptrs via `shared_from_this`; requires elements to inherit `enable_shared_from_this`.

### `Infrastructure/SharedInterfaces/TriBool.h`
- `enum class TriBool : char { False=-1, Neutral=0, True=1, Unknown=2 }` — three-valued logic used by predicates and split operators to represent positive/negative/coplanar/unknown half-space positions.
- `fmt::formatter<TriBool>` specialisation — formats as `"False"/"Neutral"/"True"/"Unknown"`.

### `Infrastructure/SharedInterfaces/PerfoTracker.h`
`namespace PBTools`

- `PerfoTimer` — single-timer: `Start()`, `Stop()` → records min/max/total/per-call log.
- `PerfoTracker(path, name)` — multi-dimensional profiling tracker; hierarchical two-key timers (`StartTimer/StopTimer`), integer/double/memory counters. **Side effect**: writes JSON/text report on destruction to `m_OutputFolder`. Not thread-safe for concurrent `StartTimer/StopTimer` on the same key pair.
- `ScopedPerfoTimer(PerfoTracker*, firstKey, secondKey)` — RAII; calls `StartTimer` on ctor, `StopTimer` on dtor. Null-safe.
- `PerfoTracker::GetUsedMem() -> size_t` (static) — current RSS.
- `PerfoTracker::GetMaxUsedMem() -> size_t` (static) — peak RSS.

### `Infrastructure/SharedInterfaces/unordered_flyweight.h`
- `unordered_flyweight<T, ...>` — alias for `boost::flyweights::flyweight<T, unordered_factory, ...>` backed by `boost::unordered_node_set`. Single-threaded use only.

### `Infrastructure/SharedInterfaces/concurrent_flyweight.h`
- `concurrent_flyweight<T, ...>` — alias for a thread-safe flyweight backed by `boost::concurrent_flat_set`. Includes a background GC thread that cleans zero-refcount entries every second. **Caveat**: GC thread joins in destructor — ensure the object outlives all users.

### `Infrastructure/SharedInterfaces/ConstantsDefinitions.h`
`namespace DefaultSettings`

Key constants (non-exhaustive):
- `DefaultSquareDistanceTolForPonctualMove = 1e-9` — squared distance below which a move is considered punctual.
- `DefaultGriddingTol = 1e-2` — default plane grid tolerance.
- `DefaultPlaneGridApproximation = 1e-5` — quantization grid step.
- `DefaultAbsoluteMinGridStepForQuantization = 1e-12` — floor grid step.
- `DefaultMaxErrorOnCartesianEvaluation = 1e-4` — max allowed error when computing Cartesian coordinates from BSP representation.

### `Infrastructure/SharedInterfaces/MultiThreadingSettings.h`
- `IsMonothreadForced() -> bool` — checks `DISABLE_MT` environment variable (cached at first call). When true, operators should run single-threaded.

---

## Module: `Mathematics/` — PB2Maths

Exact and filtered arithmetic for exact geometric predicates. All types live in `namespace PlaneBasedMathematics`.

### `Mathematics/SharedInterfaces/TypeDefinitions.h`
- `enum class Sign_t { NEGATIVE=-1, ZERO=0, POSITIVE=1, UNDEFINED=2 }` — used by all exact arithmetic types.
- `template<class NT> struct MakeVar` — factory template specialised per arithmetic type (DoubleDynamicError, IntervalArithmetic, FloatRingExtension). Methods: `FromDifference`, `FromDotProduct`, `FromPlaneEval`, `From2x2Determinant`, `From3x3Determinant`.

### `Mathematics/SharedInterfaces/DoubleDynamicError.h`
Semi-static filter: stores a double estimate `x_`, an absolute magnitude bound `m_`, and an error degree `d_`. Error bound = `(d+2) * eps * m / 2`.

- `DoubleDynamicError(double x)` — wraps an exact double; `d_=0`.
- `DoubleDynamicError(array<double,DIM> v1, array<double,DIM> v2)` — constructor for dot product; uses SIMD (xsimd) when `DIM >= simd_size`.
- `DoubleDynamicError::Sign() -> Sign_t` — returns POSITIVE/NEGATIVE/ZERO/UNDEFINED; UNDEFINED means the error interval straddles zero. **Callers must handle UNDEFINED** and fall back to IntervalArithmetic or FloatRingExtension.
- `DoubleDynamicError::Estimate() -> double`, `LowerBound()`, `UpperBound()`.
- Arithmetic: `operator+=(b)` increments `d_` by 1+max; `operator*=(b)` sums degrees. **Never overflows** unless inputs are extreme.
- Free functions: `MakeVarFrom2x2Determinant`, `MakeVarFrom3x3Determinant` — convenience factories for 2×2 and 3×3 determinants.
- `MakeVar<DoubleDynamicError>::FromPlaneEval(eq[4], pt[3])` — evaluates `ax+by+cz+d` using the dot-product constructor; exact result if coefficients are exact.

### `Mathematics/SharedInterfaces/IntervalArithmetic.h`
- `IntervalArithmetic` — typedef for `boost::numeric::interval_lib::unprotect<boost::numeric::interval<double>>::type`. Rounding mode is managed by `FloatingPointsRounding` guard; callers must ensure rounding mode is round-to-nearest or use the RAII guard.
- `SignOf<IntervalArithmetic>(interv) -> Sign_t` — uses Boost interval `cerlt/cergt/cereq` for certified comparisons.
- `ToDouble<IntervalArithmetic>(a) -> double` — returns `median(a)`.

### `Mathematics/SharedInterfaces/FloatRingExtension.h`
Exact rational arithmetic via `LargeInteger` mantissa + integer exponent. Supports `+`, `-`, `*`. **No division** (ring, not field).

- `FloatRingExtension(double)` — converts double to exact representation.
- `FloatRingExtension::Sign() -> Sign_t` — delegates to `m_Mantisse.Sign()`; always exact.
- `FloatRingExtension::Estimate() -> double`, `LowerBound()`, `UpperBound()`.
- `FloatRingExtension::compare(other) -> int` — exact comparison returning -1/0/1.
- `FloatRingExtension::hash() -> size_t` — needed for use in flyweight/unordered containers.
- **Caveat**: multiplication causes mantissa growth (big integer product); can be slow for deep expression trees. Use as last-resort fallback after DoubleDynamicError and IntervalArithmetic.

---

## Module: `PlaneBasedGeometry/` — PB2Geo

All geometry objects live in `namespace PlaneBasedGeoObjects` (aliased `PBGO`).

### `PlaneBasedGeometry/SharedInterfaces/CartesianGeometry.h`
- `CartesianGeometry::Point3D` = `Eigen::Vector3d`; `Vector3D` = same.
- `CartesianGeometry::Point2D` = `Eigen::Vector2d`.
- `CartesianGeometry::Plane` — lightweight approximation plane (origin + two axes); **not** the BSP Plane. Methods: `SetFrom3Points`, `ProjectedUV`, `ProjectedPoint`, `Eval(u,v)`, `GetNormal`, `DistanceTo`.
- `distance2(P1, P2) -> double` — squared Euclidean distance.
- `mesh_volume_integration(V, F, computeInertia) -> VolumeIntegration` — volume/CoG/inertia via tetrahedra integration.
- **Eigen caveat**: never pass `Point3D`/`Vector3D` by value from Eigen expressions; never use `auto` on Eigen results.

### `PlaneBasedGeometry/SharedInterfaces/Plane.h`
Abstract base. All BSP planes share ownership via `shared_ptr<Plane>`.

- `Plane::CreateCartesianPlane(a,b,c,d) -> shared_ptr<Plane>` — factory; returns `PlaneCartesian`.
- `Plane::Create3PointsPlane(P1,P2,P3, useCartesianIfPossible=true) -> shared_ptr<Plane>` — returns `PlaneCartesian` if the 3-point plane has a clean rational representation, otherwise `PlaneThrough3Points`.
- `Plane::GetType() -> PlaneType` — `{Cartesian, ThreePoints}`.
- `Plane::GetStdEquation_ExpDynamic() -> Equation<DoubleDynamicError>` — returns the plane coefficients `[a,b,c,d]` as `optional<array<DoubleDynamicError,4>>`.
- `Plane::GetStdEquation_ExpInterval() -> Equation<IntervalArithmetic>`
- `Plane::GetStdEquation_FloatRing() -> Equation<FloatRingExtension>`
- `Plane::GetStdEquationTpl<NT>()` — template helper dispatching to the three above.
- `Plane::GetCoords() -> array<double,4>` — **approximated** double coefficients; may not be exact for `PlaneThrough3Points`.
- `Plane::GetNormal() -> Vector3D` — approximated.
- `Plane::IsCanonical(oAxisDir) -> bool` — true for axis-aligned planes with unit normal.
- `Plane::ProjectPointOnPlane(pt) -> Point3D` — approximate Cartesian projection.
- `Plane::CreatePlaneEquation3P(P1,P2,P3) -> array<double,4>` — static helper, approximate only.

### `PlaneBasedGeometry/SharedInterfaces/PlaneCartesian.h`
`class PlaneCartesian final : public Plane`

- Stores `array<double,4> m_Doubles` = `[a,b,c,d]` of equation `aX+bY+cZ+d=0`.
- `PlaneCartesian(array<double,4>)` — direct constructor.
- `GetCoords() -> array<double,4>` — exact (no approximation).
- `SetCoords(array<double,4>)` — in-place update; used by `PlaneUpdater`.
- `MakeEquation<TReal>()` — converts doubles to any NT via cast; stored inline.
- `IsCanonical(oAxisDir) -> bool` — true if exactly one of `a,b,c = ±1` and the others are 0.

### `PlaneBasedGeometry/SharedInterfaces/PlaneThrough3Points.h`
`class PlaneThrough3Points final : public Plane`

- Stores `array<FlyWeightPt_t,3>` — three interned `Point3D` values.
- Caches `m_ExpDynamicEq` (DoubleDynamicError equation) computed at construction.
- `GetStdEquation_ExpInterval()`, `GetStdEquation_FloatRing()` — computed lazily from `MakeEquation<NT>()` using the cross-product formula.
- `Get3Points() -> array<Point3D,3>`.
- `IsCanonical()` — always returns false.
- **Gotcha**: `GetCoords()` (inherited, double) will call `MakeEquation<double>()` giving an approximation.

### `PlaneBasedGeometry/SharedInterfaces/Point.h`
Abstract base. Points always carry references to 3 supporting planes.

- `Point::CreatePoint(P1,P2,P3, oIsValid, checkForExactCartesian) -> shared_ptr<Point>` — main factory; returns `PointCartesian` if the intersection is exactly representable as Cartesian, otherwise `PointBy3Planes`. Sets `oIsValid=false` if determinant is zero.
- `Point::CreatePoint(P1,P2,P3, cartesianPt) -> shared_ptr<Point>` — hybrid: trusts the caller that `cartesianPt` lies exactly on all three planes.
- `Point::CreatePoint(cartesianPt) -> shared_ptr<Point>` — pure Cartesian point; creates 3 canonical axis planes internally.
- `Point::IsIntersectionExactCartesian(planes[3]) -> pair<bool,Point3D>` — checks if the 3-plane intersection is representable exactly as a double-coordinate point.
- `Point::GetType() -> PointType` — `{ThreePlanes, Cartesian}`.
- `Point::GetP1/P2/P3() -> const Plane*` — raw pointers to the 3 supporting planes.
- `Point::GetP1/P2/P3Shrd() -> shared_ptr<Plane>`.
- `Point::GetCartesianRepresentation_ExpDynamic() -> Equation<DoubleDynamicError>` — 4-element array `[X,Y,Z,1]` in DoubleDynamicError.
- `Point::GetCartesianRepresentationTpl<NT>()` — template dispatch.
- `Point::GetApproximatedPoint() -> Point3D` — approximate Cartesian xyz.
- `Point::UpdatePlane(pos, newPlane) -> bool` — replaces one of the 3 planes; returns false if the updated point is no longer valid.
- `Point::SetQuietMode(bool)` / `GetQuietMode()` — static; suppresses warnings when quiet=true.

### `PlaneBasedGeometry/SharedInterfaces/PointBy3Planes.h`
`class PointBy3Planes final : public Point`

- Stores `m_DynCache` (DoubleDynamicError equation) computed at construction via `Init()`.
- `Init() -> bool` — computes 3×3 determinant; returns false if degenerate (sets `oIsValid`).
- `GetCartesianRepresentation_ExpDynamic()` — returns cache directly (no recomputation).
- `GetCartesianRepresentation_ExpInterval()` — computes lazily via `MakeEquation<IntervalArithmetic>()`.
- `GetCartesianRepresentation_FloatRing()` — computes lazily via `MakeEquation<FloatRingExtension>()`.
- `GetApproximatedPoint()` — evaluates DoubleDynamicError and converts to double.
- **Caveat**: the `m_DynCache` may be `std::nullopt` if the determinant is zero at construction; always check via `GetType()` before use.

### `PlaneBasedGeometry/SharedInterfaces/PointCartesian.h`
`class PointCartesian final : public Point`

- Stores `FlyWeightPt_t m_XYZPoint` — interned `Eigen::Vector3d`.
- Created with 3 canonical axis planes matching the XYZ coordinates.
- `GetApproximatedPoint() -> Point3D` — returns the interned Cartesian value directly (exact).
- `MakeEquation<NT>()` — returns `[NT(x), NT(y), NT(z), NT(1)]`.
- **Gotcha**: the stored planes are canonical axis planes, not the planes from which the point was originally computed; `GetP1/P2/P3()` will NOT return the original domain planes.

### `PlaneBasedGeometry/SharedInterfaces/Edge.h`
`class Edge`

Value-type (copyable). Represents one directed edge in a `PolygonLoop`.

- `Edge(pts[2], polygon*, orientation, edgePlane, position={-1,-1}, nu=1)`
- `GetBeginPt() / GetEndPt() -> const Point*`
- `GetEdgePlane() -> const Plane*` — the plane containing this edge (the shared plane between two adjacent polygons).
- `GetAssociatedPolygon() -> const Polygon*` — the neighbour polygon on the other side of this edge.
- `GetEdgeOrientation() -> char` — `+1` or `-1` relative to the supporting polygon's normal.
- `GetNu() -> int` — edge multiplicity (1 = normal, >1 = degenerate/multi-intersection).
- `SwapExtremities()` — swaps begin/end in place.
- `empty() -> bool` — true if either endpoint is null.
- `IsPoint() -> bool` — true if both endpoints are the same.

### `PlaneBasedGeometry/SharedInterfaces/PolygonLoop.h`
`class PolygonLoop final`

Closed chain of points + edge-planes + neighbour polygon pointers + edge orientations. All vectors use `boost::container::small_vector` with `kSBO=8` (inline storage for ≤8-sided polygons).

- `PolygonLoop(boundaryEdges, points, orientations, polygons)` — main constructor.
- `GetNbEdges() / GetNbPoints() -> size_t` — equal (closed loop).
- `GetEdge(i) -> const Plane*` — i-th edge plane.
- `GetPt(i) -> const Point*` — i-th vertex.
- `GetAssociatedPolygon(i) -> const Polygon*` — neighbour across edge i.
- `GetNextIdx(i) / GetPrevIdx(i) -> size_t` — ring-safe index arithmetic (ternary, ~5-8× faster than modulo).
- `GetBeginPtIdxFromEdgeIdx(i)` / `GetEndPtIdxFromEdgeIdx(i)` — vertex/edge index convention: vertex i is at intersection of edge i and edge i+1 (vertex i = end of edge i = start of edge i+1).
- `SplitEdge(index, newPoint)` — inserts a point on edge i; updates all parallel vectors.
- `ReverseLoop() -> ErrorKinds` — reverses vertex order (changes orientation).
- `FindPseudoEdge(pts[2], coordMod) -> int` — returns edge index or -1.

### `PlaneBasedGeometry/SharedInterfaces/Polygon.h`
`class Polygon final : public enable_shared_from_this<Polygon>`

- `Polygon(supportingPlane, externalLoop)` — main constructor.
- `Polygon(supportingPlane, loops)` — with holes.
- `GetSupportingPlane() -> const Plane*`
- `GetSupportingPlaneShrd() -> shared_ptr<Plane>`
- `GetExternalLoop() -> PolygonLoop&` — first loop (external boundary).
- `GetLoop(i)` / `GetAllLoops()` — access all loops.
- `GetNbLoops() -> size_t`; `HasInternalLoop() -> bool`
- `GetTotalNbEdges() -> size_t` — sum over all loops.
- `CollectAllEdges() -> vector<Edge>`
- `CollectAllNeighbors() -> vector<Polygon*>` — **may contain duplicates**.
- `FindPseudoEdge(pts[2], coordMod) -> array<int,2>` — `{loopIdx, edgeIdx}` or `{-1,-1}`.
- `FindPoint(pt) -> optional<array<size_t,2>>` — `{loopIdx, ptIdx}`.
- `ReverseAllLoopsRotations() -> ErrorKinds`
- `GetBoundingBox() -> Box3D` — computed from approximate Cartesian point coords.
- `SetSupportingPlane(newPlane)` — in-place plane replacement; used by coplanar merging.
- `AddLoop(internalLoop) -> ErrorKinds` — adds a hole loop.
- `GetShrdPointer() -> shared_ptr<Polygon>` — `shared_from_this()`.
- Attribute management: `CreateOrGetAttributeGroup()`, `SetColor/GetColor`.

### `PlaneBasedGeometry/SharedInterfaces/Polyhedron.h`
`class Polyhedron final`

Core container: `MapOfPolygonsToOrientations_t` = `boost::unordered_node_map<shared_ptr<Polygon>, char, polygon_hash, equal_to<>>`. Orientation `char` = `+1` (normal out) or `-1` (normal in).

- `Polyhedron()` — default empty.
- `Polyhedron(MapOfPolygonsToOrientations_t)` — move-construct from pre-built map.
- `Polyhedron(vector<shared_ptr<Polyhedron>>)` — concatenation constructor.
- `AddAPolygon(polygon, orientation)` — adds one polygon; invalidates bounding box + topology cache.
- `RemoveAPolygon(polygon) -> bool` — returns true if found and removed.
- `AddPolygons(first, last)` — range insert from map-compatible iterators.
- `AddPolygons(first, last, orientation)` — range insert from polygon container.
- `GetNbPolygons() -> size_t`; `size()`, `empty()`, `begin()/end()`.
- `GetAllPolygonsAndOrientations() -> MapOfPolygonsToOrientations_t&` — non-const access invalidates bbox and topology cache.
- `GetBoundingBox() -> Box3D` — lazily computed; cached; invalidated by any mutation.
- `CollectAllPoints() -> parallel_flat_hash_set<shared_ptr<Point>>` — expensive O(N×edges) traversal.
- `GetOrBuildTopologyGraph() -> PolyhedronTopologyGraph&` — lazily builds/returns the topology graph; cached; invalidated by mutations. **Thread-safety**: not thread-safe during build.
- `InvalidateTopologyGraph()` / `ReleaseTopologyGraph()` — manual invalidation.
- `IsTopologyGraphCacheDirty() -> bool`

### `PlaneBasedGeometry/SharedInterfaces/PolyhedronTopologyGraph.h`
`class PolyhedronTopologyGraph final`

Undirected Boost.Graph (`adjacency_list<vecS,vecS,undirectedS>`) over polygon adjacency.

- Vertex data: `TopologyGraphVertexData { Polygon*, orientation, visited, classification }`.
- Edge data: `TopologyGraphEdgeData { SharedEdgeCount, IsCutBlocked }`.
- `Build(polygons: vector<OrientedPolygon_t>)` — full O(N·E_per_poly) build; clears existing graph.
- `Validate(polygons) -> TopologyGraphValidationResult` — checks structural consistency.
- `ConvertToVector<TMap>(map) -> vector<OrientedPolygon_t>` — static helper to convert Polyhedron map.
- `FindVertex(polygon) -> optional<Vertex_t>` — O(1) via `m_PolygonToVertex` flat_hash_map.
- `MarkCutEdge(polyA, polyB) -> bool` — marks the edge between two polygons as a cut; returns false if no such edge.
- `ClearCutEdgeMarks()` — resets all `IsCutBlocked` flags.
- `IsCutEdgeBlocked(vertex) -> bool` — true if any adjacent edge is cut-blocked.
- `GetGraph() -> const Graph_t&` — raw Boost graph access.

### `PlaneBasedGeometry/SharedInterfaces/GeoObjectFactory.h`
`class GeoObjectFactory` — abstract interface.

- `SetGriddingTolerance(worldBox, tol) -> ErrorKinds` — sets the quantization grid for plane canonicalisation.
- `CreatePlane(eq[4], oHasBeenFlipped, options) -> shared_ptr<Plane>` — options bitmask: `Exactly=0`, `AllowFlipping=1`, `AllowQuantization=2`. Returns an existing plane if the quantized/flipped equation matches a previously created one. **Gotcha**: `oHasBeenFlipped` must be checked to correct polygon orientation.
- `CreatePoint(planes[3], oIsValid) -> shared_ptr<Point>` — canonical 3-plane intersection; deduplicates via the factory's internal map.
- `CreatePoint(P1,P2,P3, oIsValid)` — variant with individual plane args.
- `CreatePoint(cartesianPt) -> shared_ptr<Point>` — Cartesian point.
- `CreatePoint(P1,P2,P3, cartesianPt) -> shared_ptr<Point>` — hybrid; the factory may ignore the planes if the Cartesian point already exists.
- `CreatePointOnEdge(planes[3], pt1, pt2, plane) -> shared_ptr<Point>` — creates a point that lies on the edge between pt1 and pt2 on the given plane; used during splitting.
- `Size() -> size_t` — number of stored elements.
- `Tass() -> ErrorKinds` — compacts the factory by removing elements referenced only by the factory itself.
- `QuantizePlaneEquation(normal)` — deprecated; use `CreatePlane` with `AllowQuantization` instead.

### `PlaneBasedGeometry/SharedInterfaces/Shapes.h`
Primitive polyhedron/polygon builders used by tests, fixtures, and importer sanity checks.

- `Shapes::MakeFromAABB(iLow: Point3D, iHigh: Point3D, iObjectFactory: GeoObjectFactory* = nullptr) -> shared_ptr<Polyhedron>` — axis-aligned box helper; forwards through `MakeFromOBB` so face loops come back with outward-facing orientation.
- `Shapes::MakeFromOBB(iOBB: OBox3D, iObjectFactory: GeoObjectFactory* = nullptr) -> shared_ptr<Polyhedron>` — oriented box builder; creates the six support planes from the OBB basis and flips loops/orientation flags when the factory canonicalizes a plane.
- `Shapes::MakeHexahedron(bottom, up, right, left, near, far, iObjectFactory: GeoObjectFactory* = nullptr) -> shared_ptr<Polyhedron>` — manual six-plane box builder. **Gotcha**: argument order is geometric (`z-`, `z+`, `y+`, `y-`, `x-`, `x+` in canonical axes), not arbitrary; swapping planes changes the signed volume.
- `Shapes::MakeTetrahedron(facePlanes[4], iObjectFactory: GeoObjectFactory* = nullptr) -> shared_ptr<Polyhedron>` — tetrahedron from four outward-facing planes. The implementation reverses the generated face loops so polygon winding matches the face-plane normals and mass properties stay positive.
- `Shapes::MakeTetrahedron(vertices[4]) -> shared_ptr<Polyhedron>` — tetrahedron from four Cartesian vertices; the first three generated face loops are reversed to keep the signed area/volume consistent with the outward face normals.
- `Shapes::MakeCone(iConeApex: Point3D, iConeAxis: Vector3D, iRadius: double, iNbSections: uint32_t) -> shared_ptr<Polyhedron>` — cone builder with a canonical-axis fast path and a generic sampled-base path for oblique axes. The generic branch derives its winding sign from the sampled base-plane normal versus `iConeAxis`, so negative oblique axes keep positive signed volume.

---

## Module: `PlaneBasedOperators/` — PB2Operators

### `PlaneBasedOperators/SharedInterfaces/BooleanCommonTypes.h`
`namespace BooleanCommonTypes`

- `OrientedPolygon_t = pair<Polygon*, char>` — raw pointer + orientation; used in hot paths.
- `SharedOrientedPolygon_t = pair<shared_ptr<Polygon>, char>` — ownership-retaining variant.
- `TwoOrientedPolygons_t = array<OrientedPolygon_t,2>` — one pair of potentially intersecting polygons.
- `PolygonToFragmentsJournal_t = tbb::concurrent_unordered_map<OrientedPolygon_t, concurrent_vector<SharedOrientedPolygon_t>, ...>` — maps each split polygon to its fragments; thread-safe via TBB.
- `hash_OrientedPolygon_t` — hashes by `Polygon*` pointer only (ignores orientation).
- `ShrdOrientedToPtr / ShrdToVectorOfPtr` — conversion utilities.

### `PlaneBasedOperators/SharedInterfaces/BooleanOperator.h`
`class BooleanOperator` — abstract interface; non-copyable.

- `SetFirstOperand(polyhedron)` / `SetSecondOperand(polyhedron)`
- `Run(BooleanKind) -> ErrorKinds`
- `enum BooleanKind { Union, Intersection, Difference, FragmentOnly, SymetricDifference }`
- `SetPerfoSpy(PerfoTracker*)` — optional; no-op default.
- `SetSuperGraphLocalizer(shared_ptr<SuperGraphTagLocalizer>)` — optional; no-op default.
- `GetModifications(oRemovedPolygons, oNewPolygons) -> ErrorKinds` — call after `Run` to retrieve the diff.

### `PlaneBasedOperators/LocalInterfaces/BooleanOperatorImpl.h`
`class BooleanOperatorImpl final : public BooleanOperator` — concrete Boolean pipeline.

- `BooleanOperatorImpl(pairFinder)` — takes ownership of the pair-finder strategy.
- `Run(BooleanKind) -> ErrorKinds` — full pipeline:
  1. `GetIntersectingPairs` via m_PairFinder
  2. `SeekCoplanars` — identifies coplanar polygon pairs
  3. `OptimizeCoplanars` — replaces B's coplanar planes with A's
  4. `ComputeIntersections` — polygon-polygon line intersections (parallel via TBB)
  5. `ChainIntersectionSegments` → polylines
  6. `SplitPolygonsByPolyLines` → fragment journals
  7. `ReplacePolygonsByFragments` — updates both operand polyhedra
  8. `TagAndSortPolygons` — runs `TagAndSortPolygonsOpe`
- `GetModifications(oRemoved, oNew) -> ErrorKinds`
- `SetSuperGraphLocalizer(localizer)` — attaches persistent super-graph for accelerated tag diffusion.
- **Caveat**: operands are mutated in place. The caller retains ownership via `shared_ptr`.

### `PlaneBasedOperators/LocalInterfaces/TagAndSortPolygonsOpe.h`
`class TagAndSortPolygonsOpe` — tags each polygon fragment as Inside/Outside the other operand and collects add/remove lists.

- `TagAndSortPolygonsOpe(polyhedrons[2], polylines[2], fragments[2], intersectingPairs, booleanKind)`
- `SetPairFinder(pairFinder)` — injects the spatial localizer used by point-in-polyhedron seeding queries. If it is not set, `IsPointInsidePolyhedron` now falls back to a full polygon scan (with a warning) instead of dereferencing a null strategy.
- `SetSuperGraphLocalizer(localizer)` — attaches persistent super-graph to accelerate BFS tag diffusion.
- `SetUseParallelTagDiffusion(bool)` / `SetParallelTagDiffusionThreads(int)` — enable TBB parallel BFS.
- `Run() -> ErrorKinds` — executes tagging pipeline.
- `GetPolygonsToAdd() -> const vector<OrientedPolygon_t>&`
- `GetPolygonsToRemove() -> const vector<OrientedPolygon_t>&`
- `enum WNType { InOther=0, OutOther=1 }` — winding number classification.
- Internal strategies (private): `SpreadTag`, `SpreadTagWithGraph` (topology-graph BFS), `SpreadTagWithParallelDiffusion` (TBB BFS). Strategy chosen based on availability of topology graph and parallel diffusion flags.
- `IsPointInsidePolyhedron(polyhedron, point) -> TriBool` — ray-casting fallback for seeding.

### `PlaneBasedOperators/SharedInterfaces/PolylineSegmentsChaining.h`
`class PolylineSegmentsChaining` — chains unordered split/intersection segments into closed or open `PBGO::Polyline` objects.

- `PolylineSegmentsChaining::Run(iUseGeometricComparison: bool = false) -> ErrorKinds` — groups segment connectivity, runs component extraction, and materialises the chained polylines. Returns immediately with an empty result when no segments are provided, instead of calling `boost::connected_components()` on an empty graph.
- `PolylineSegmentsChaining::GetResult() -> vector<PBGO::Polyline>&&` — moves the chained polylines out after `Run()`.

### `PlaneBasedOperators/SharedInterfaces/SuperGraphTagLocalizer.h`
`class SuperGraphTagLocalizer` — persistent super-graph for O(N_supernodes) tag diffusion. Non-copyable (DISALLOW_COPYCTOR_ASSIGN_AND_MOVE).

- `Init(operand)` — full BFS clustering into fixed-size super-nodes (`kClusterSizeMin=64`, `kTargetSuperNodeCount=1024`). Skipped if `!m_NeedsRebuild`.
- `UpdateLocalizer(removed, added)` — incremental update after a Boolean operation; triggers full rebuild if delta fraction > `kRebuildThreshold=0.35` or polygon count drops to zero.
- `EvictPolygons(removed, added)` — lightweight pointer-swap used BEFORE `ReplacePolygonsByFragments` to prevent dangling raw pointers; does NOT rebuild adjacency.
- `ApplyCutEdges(cutEdges)` — per-operation overlay: marks `m_IsBoundary`/`m_HasCutEdge` on affected super-nodes.
- `ClearCutEdgeOverlay()` — resets overlay after each `Run()`.
- `FindSuperNode(polygon) -> int` — O(1) lookup; returns -1 if not found.
- `GetSuperNodes() -> const vector<SuperNodeHot>&` — hot BFS data (id, adjacency, tag fields).
- `GetMemberLists() -> const vector<SuperNodePolygons_t>&` — cold data (polygon pointers per super-node).
- `IsInitialised() -> bool` / `GetSuperNodeCount() -> size_t`
- `Invalidate()` — force rebuild on next `Init()`.
- **SuperNodeHot fields**: `m_Id`, `m_Adjacent` (small_vector<int,8>), `m_IsBoundary`, `m_HasCutEdge`, `m_Tag` (0=unset, 1=InOther, -1=OutOther), `m_IsSeeded`.
- **Lifecycle**: create once per operand polyhedron; attach via `BooleanOperator::SetSuperGraphLocalizer`; caller keeps alive across multiple `Run()` calls.

### `PlaneBasedOperators/SharedInterfaces/FindPairsOfPolygonsToIntersect.h`
`class FindPairsOfPolygonsToIntersect` — abstract pair-finder interface (non-copyable).

- `SetFirstOperand / SetSecondOperand(polyhedron)`
- `GetIntersectingPairs() -> deque<TwoOrientedPolygons_t>` — default returns all O(N×M) pairs.
- `GetFirstOperandPolygonsCrossingPlane(plane) -> deque<OrientedPolygon_t>` — default returns all first operand polygons.
- `UpdateLocalizer(toRemove, toAdd)` — no-op default.
- Concrete implementations: `RTreeSpatialPolyhedronsIntersectionsLocalizer`, `EmbreeLBVHSpatialPolyhedronsIntersectionsLocalizer`.

### `PlaneBasedOperators/SharedInterfaces/RTreeSpatialPolyhedronsIntersectionsLocalizer.h`
`class RTreeSpatialPolyhedronsIntersectionsLocalizer : public FindPairsOfPolygonsToIntersect`

- `RTreeSpatialPolyhedronsIntersectionsLocalizer(worldBox)` — builds the R-Tree from bounding boxes.
- `GetIntersectingPairs()` — spatial query; parallel via TBB when `m_EnableParalel=true`.
- `UpdateLocalizer(toRemove, toAdd)` — incremental update; reorders input vectors.
- `GetAllNonEmptyNodesBoxes() -> vector<OBox3D>` — diagnostics.
- `SetRTreeDebugActive(bool)` — static toggle for verbose R-Tree logs.

### `PlaneBasedOperators/SharedInterfaces/SplitPolygonByPlaneOpe.h`
`class SplitPolygonByPlaneOpe` — splits one polygon by a cutting plane.

- `SplitPolygonByPlaneOpe(cuttingPlane, polygon)` — non-copyable.
- `Run() -> ErrorKinds`
- `GetResult(oNeg, oPos, oCoPlanar, oPolyLine) -> TriBool` — returns:
  - `TriBool::Negative` — polygon is entirely on negative side.
  - `TriBool::Positive` — entirely on positive side.
  - `TriBool::Neutral` — coplanar.
  - `TriBool::Unknown` — split occurred (oNeg/oPos populated).

### `PlaneBasedOperators/SharedInterfaces/SplitPolygonByPolylineOpe.h`
`class SplitPolygonByPolylineOpe` — splits one polygon by one or more cutting polylines already embedded in the polygon support plane.

- `Run() -> ErrorKinds` — inserts polyline edges into all touched loops, reconstructs candidate loops, classifies them as external or internal, and materialises the split result polygons.
- **Gotcha**: a cutting polyline can touch more than one loop (for example an external boundary plus a hole). The implementation must mark **every** touched loop as intersected before reconstructing untouched carry-over loops; otherwise stale internal loops survive unsplit and area conservation fails.
- Loop-orientation disambiguation that remains after touch-analysis is expected in some cases; the current implementation only logs the remaining `LoopsToDetermine` count at debug level instead of treating it as an error by itself.

### `PlaneBasedOperators/LocalInterfaces/PolygonsMerger.h`
`namespace PlaneBasedOperators::algorithms`

- `MergePolygons(polygonsToMerge) -> shared_ptr<PBGO::Polygon>` — merges one connected coplanar polygon set by collecting boundary edges, chaining them into closed loops, then rebuilding one polygon. When several closed loops remain, the implementation selects the external loop from the globally lexicographically-maximal loop point using `AdvPredicates::LexicographicOrder(...)`, avoiding approximate loop-area accumulation in the outer/inner decision.
- `ReplacePlaneInPolygons(polygonToSimplify, newPlane, oldPlanes)` — rewrites only those loop edge planes / point planes that actually belong to `oldPlanes`. The edge replacement must use each edge’s own shared plane, not the polygon support plane, otherwise simplification can collapse merged importer faces into degenerate geometry.
- `SimplifyPolygon(polygon)` — removes colinear useless points from a polygon loop while preserving neighbour references.

### `PlaneBasedOperators/SharedInterfaces/TriMeshImporter.h`
`class TriMeshImporter` — abstract importer from triangle soup (`Eigen::MatrixXd` + `Eigen::MatrixXi`) to `PBGO::Polyhedron`.

- `Simplification(bool)` — enables the coplanar-triangle merge pass before `Run()`.
- `Run() -> ErrorKinds` — builds supporting planes, loop edges/points, optional simplification, then materialises the output polyhedron.
- **Implementation note (`TriMeshImporterImpl`)**: the simplification equivalence graph must only connect adjacent triangles whose support planes are both coincident **and** equally oriented. Merging non-coplanar or opposite-oriented neighbours causes catastrophic over-merge in mesh-import regression cases.
- **Implementation note (`TriMeshImporterImpl`)**: `MergePolygons(...)` may fail and return `nullptr` for a subgroup; the importer now skips those groups instead of feeding null polygons into removal, neighbour-glue, or plane-replacement passes.

### `PlaneBasedOperators/LocalInterfaces/UpdateNeighboorLink.h`
- `TopologyHelpers::GlueBoundaryPolygons(pairOfPolygonVectors)` — glues shared boundary edges after split/merge steps. The helper now drops null polygon pointers before sorting/deduplicating neighbour candidates, which prevents null-neighbour crashes after failed importer merge attempts.

### `PlaneBasedOperators/SharedInterfaces/FactoryOfOperators.h`
`namespace FactoryOfOperators`

- `CreatePBBooleanOper(pairFinder) -> unique_ptr<BooleanOperator>` — creates a `BooleanOperatorImpl`; caller provides the spatial localizer strategy.
- `CreateTriMeshImporter(Vertices, Faces) -> unique_ptr<TriMeshImporter>` — creates a `TriMeshImporterImpl` from Eigen mesh arrays.

---

## Module: `PlaneBasedIO/` — PB2IO

### `PlaneBasedIO/SharedInterfaces/PBIO.h`
`namespace PlaneBasedIO`

- `ReadMesh(inputFile) -> pair<Eigen::MatrixXd, Eigen::MatrixXi>` — reads any Assimp-supported mesh format; returns `(V, F)` matrices (vertices N×3, faces M×3).
- `WriteMesh(outputFile, format, V, F, optionalColor)` — writes via Assimp. `format` is an Assimp exporter ID string (e.g. `"obj"`, `"stl"`). `EigenColorMatrix_t = Matrix<unsigned char, Dynamic, Dynamic>` for per-vertex colors.

---

## Module: `PlaneBasedTests/` — PB2Test

Main regression Boost.Test executable. Top-level suites are `MathematicsTests`, `PlaneBasedGeometryTests`, `PlaneBasedOperatorsTests`, `PlaneBasedIOTests`, and `InfrastructureTests`; `PlaneBasedTests/CMakeLists.txt` registers matching module-level `ctest` entries.

| File | What it tests |
|---|---|
| `src/main.cpp` | `BOOST_TEST_MODULE PB2Test`; installs the logger-backed Boost formatter/observer so `BOOST_TEST_MESSAGE`, warnings, and failures flow through `Logger.h` |
| `TestExactArithmetic.cpp` | DoubleDynamicError, IntervalArithmetic, FloatRingExtension |
| `TestPBPredicates.cpp`, `TestPBPredicatesP3P.cpp`, `TestGeoEvaluators.cpp`, `TestPredicatesEngine.cpp` | filtered/exact predicate stacks and geometric predicate adapters |
| `TestBooleansSimpleCases.cpp`, `TestBooleansSingularCases.cpp`, `TestSegmentsBooleans.cpp`, `TestTopologyGraphDualMode.cpp` | Boolean regression coverage, including singular/coplanar cases and topology-graph diffusion parity |
| `TestSplitPolygonByPlane.cpp` | SplitPolygonByPlaneOpe |
| `TestSplitPolygonByPolylines.cpp` | SplitPolygonByPolylineOpe |
| `TestPolyhedronTopologyGraph.cpp` | PolyhedronTopologyGraph build/validate |
| `TestTriMeshImportAndMerge.cpp` | retained mesh-import+merge regression subset: `CubeTroue`, `MengerSponge_0`, `MengerSponge_1`, `MengerSponge_2` |
| `TestTriMeshImporter.cpp` | retained mesh-import regression subset: `PB2Import_Tetra`, `PB2Import_Pad12288T`, `PB2Import_ToreCreux_r25_35_R90_OBB_1183868T` |
| `TestSpatialLocalizerComparison.cpp` | correctness-only BSP-tree vs RTree overlap/empty-operand checks; the timing benchmark moved to `PB2SlowTest` |
| `TestParallelTagDiffusion.cpp` | unit tests for parallel BFS tag diffusion |
| `TestParallelTagDiffusionIntegration.cpp` | integration tests: parallel vs sequential agreement |
| `TestRTree.cpp` | RTree spatial queries |
| `TestAssimpImportExport.cpp` | PBIO read/write round-trip |
| `TestPointInPolyhedronPredicates.cpp` | inside/outside classification |

### `PlaneBasedTests/src/main.cpp`
- `BOOST_GLOBAL_FIXTURE(GlobalSetup)` — installs `LoggerTestObserver` and a Boost log formatter that forwards `BOOST_TEST_MESSAGE`, info, warnings, and errors through `Logger.h`; keeps PB2Test console/log output aligned with the rest of the project.

### `PlaneBasedTests/LocalInterfaces/SpatialLocalizerComparisonFixture.h`
- `SpatialLocalizerTestFixture::CreateTestCube(iX, iY, iZ, iWidth, iHeight, iDepth) -> shared_ptr<Polyhedron>` — builds a 6-face axis-aligned polyhedron from Cartesian planes and `Shapes::MakePolygon`; shared by the correctness suite in `PB2Test` and the timing comparison in `PB2SlowTest`.

### `PlaneBasedTests/LocalInterfaces/TestUtilities.h`
`namespace TestUtilities`

- `CheckPolyhedron(P) -> ErrorKinds` — asserts edge multiplicity, no null planes/points.
- `CheckPolygonArea(in, neg, pos) -> ErrorKinds` — area conservation after split.
- `RunAndCheckBoolean(name, P1, P2, kind, ref, localizerType) -> int` — runs Boolean op, checks result against `TstPolyhedronRefController` reference. Returns 0 on success.
- `CheckComputeBooleanFromMeshFiles(paths, kind, oResult, spy, localizerType) -> ErrorKinds` — imports each mesh, computes reference properties, runs the requested Boolean pipeline, optionally updates/export-profiles through `PerfoTracker`, and returns the output polyhedron for caller-side export/checks.
- `CreateSquarePolygon(plane, size) -> shared_ptr<Polygon>`
- `CreateRandomOBoxPolyhedron(center, diag) -> shared_ptr<Polyhedron>`
- `CreateLocalizer(localizerType, worldBox) -> shared_ptr<FindPairsOfPolygonsToIntersect>`
- `TestPBMeshImporter(path, merge, closeGap, spy) -> int` — reads a mesh via `PlaneBasedIO::ReadMesh`, computes reference area/volume/CoG from the source triangles, runs `TriMeshImporter`, applies `Simplification(merge)`, exports the resulting polyhedron, and compares geometry against the original mesh. `merge=true` cases are the heavier import+merge regressions reused by `PB2SlowTest`.

---

## Module: `PerformanceTests/` — PerformanceTests CLI + PB2SlowTest

Holds the command-line performance harness and the slow Boost.Test executable used for long-running or benchmark-style scenarios. `PerformanceTests/CMakeLists.txt` builds both binaries; default `ctest` registration is intentionally limited to `ImportAndBoolean`, `ImportAndBooleanPerf`, and `SpatialLocalizerComparisonTests`.

| File | What it tests / drives |
|---|---|
| `src/main.cpp` | CLI entry point for iteration-based performance runs (`--test`, `--iteration`, `--output_dir`, optional `--file`, `--localizer`) |
| `src/TestsPerformance.h`, `src/TestsPerformance.cpp` | benchmark driver API and helper layer for repeated Boolean/import measurements |
| `src/SlowTestMain.cpp` | `BOOST_TEST_MODULE PB2SlowTest`; installs the same logger-backed Boost formatter used by `PB2Test` |
| `src/TestBooleansPerformance.cpp` | `PerformanceTestsSuite/ImportAndBoolean` and `ImportAndBooleanPerf`: heavy imported-mesh Boolean scenarios with export + `PerfoTracker` output |
| `src/TestTriMeshImporter.cpp` | slow importer-only or importer+merge cases moved out of `PB2Test`: `PB2Import_Pad12T`, `PB2Import_TREX`, `Ganesha` |
| `src/TestTriMeshImportAndMerge.cpp` | slow mesh-import+merge cases moved out of `PB2Test`: `MengerSponge_3`, `One_Segment`, bug repro meshes, `T_REX`, `steamdeck`, `Medusa`, `Torture3DPrint` |
| `src/TestSpatialLocalizerPerformance.cpp` | benchmark-style BSP-tree vs RTree timing comparison using the shared `SpatialLocalizerTestFixture` |

### `PerformanceTests/src/TestsPerformance.h`
- `enum class LocalizerType { Default, RTree, Spatial, Temporary }` — selector shared by the CLI performance harness. `Default` currently routes through the generic/default pair-finder path.
- `TestPerformance::Rotating_Cube_Union(outputPath, iterations, localizerType) -> int`
- `TestPerformance::Torus_Cube_Union(outputPath, iterations, localizerType) -> int`
- `TestPerformance::Rotating_Cube_UnionOf2(outputPath, iterations, localizerType) -> int`
- `TestPerformance::Rotating_Cube_IntersectionOf2(outputPath, iterations, localizerType) -> int`
- `TestPerformance::Rotating_Cube_Intersection(outputPath, iterations, localizerType) -> int`
- `TestPerformance::Import(inputFile, outputPath) -> int` — standalone importer benchmark used by the CLI `Import` mode.

### `PerformanceTests/src/main.cpp`
- `stringToLocalizerType(str) -> LocalizerType` — accepts `rtree`, `spatial`, `temporary`, or `default` (case-insensitive for the initial capitalised forms); throws on unknown values.
- `main(argc, argv) -> int` — parses CLI options, creates the output directory/log file, dispatches to `TestPerformance::*`, and enforces that `Import` also receives `--file`.

### `PerformanceTests/src/TestsPerformance.cpp`
- `internal::CreateLocalizer(localizerType, worldBox) -> shared_ptr<FindPairsOfPolygonsToIntersect>` — returns either the generic pair finder, `RTreeSpatialPolyhedronsIntersectionsLocalizer`, or a reusable `SpatialPolyhedronsIntersectionsLocalizer` configured with leaf size 64 and max depth 16.
- `internal::ComputeMeshMassProperties(V, F) -> GeometricProperties::Properties` — computes area/volume/CoG/inertia from raw triangle meshes for importer benchmarks.
- `internal::ComputeBoolean2(P1, P2, kind, localizer, spy, sgLocalizer) -> ErrorKinds` — repeated-Boolean benchmark driver around `FactoryOfOperators::CreatePBBooleanOper`; logs polygon deltas and updates the reusable localizer between iterations. **Gotcha**: if `UpdateLocalizer()` throws, the harness logs the exception and returns `PB_OK` so benchmark runs do not abort on localizer-update failures.

### `PerformanceTests/src/SlowTestMain.cpp`
- `BOOST_GLOBAL_FIXTURE(GlobalSetup)` — mirrors `PlaneBasedTests/src/main.cpp` so `PB2SlowTest` emits the same logger-integrated Boost.Test output.

---

## Module: `PBCLI/`

### `PBCLI/src/main.cpp`
Command-line driver. Uses Boost.ProgramOptions.

- Loads one or more mesh files via `PlaneBasedIO::ReadMesh` and `FactoryOfOperators::CreateTriMeshImporter`.
- Runs Boolean operations (`Union/Intersection/Difference`) via `FactoryOfOperators::CreatePBBooleanOper` with `RTreeSpatialPolyhedronsIntersectionsLocalizer`.
- Exports results via `PolyhedronExporter`.
- Supports batch processing of directories (up to `SubdirSize=1000` files per subdir).
- Uses `indicators` library for progress bars.
- Uses Poco (`Poco::Zip::Decompress`) for zipped input archives.

---

## Key Type Aliases & Cross-Module Conventions

| Alias | Definition | Where used |
|---|---|---|
| `PBGO` | `namespace PlaneBasedGeoObjects` | everywhere |
| `PBOP` | `namespace PlaneBasedOperators` | operators, CLI |
| `OrientedPolygon_t` | `pair<Polygon*, char>` | `BooleanCommonTypes` |
| `MapOfPolygonsToOrientations_t` | `boost::unordered_node_map<shared_ptr<Polygon>,char,…>` | `Polyhedron` |
| `FlyWeightPt_t` | `unordered_flyweight<CartesianGeometry::Point3D>` | `CartesianPointFlyweight.h` |
| `PolygonToFragmentsJournal_t` | `tbb::concurrent_unordered_map<OrientedPolygon_t, concurrent_vector<SharedOrientedPolygon_t>, …>` | Boolean pipeline |
| `Sign_t` | `enum class {NEGATIVE,ZERO,POSITIVE,UNDEFINED}` | all arithmetic types |

### Exact-arithmetic fallback chain
All predicates follow this cascade:
1. **DoubleDynamicError** (fast semi-static filter) → if `Sign() == UNDEFINED` →
2. **IntervalArithmetic** (interval filter, needs rounding mode guard) → if `UNDEFINED` →
3. **FloatRingExtension** (exact, arbitrary precision) → always definite.

### Ownership model
- `Plane` and `Point` objects are always owned via `shared_ptr`; raw pointers used only in hot loops (never stored long-term).
- `Polygon` inherits `enable_shared_from_this`; always heap-allocated via `make_shared`.
- `Polyhedron` is value-copyable (contains `shared_ptr<Polygon>` map, so copy is shallow).
- `GeoObjectFactory` is abstract; concrete instances are typically owned by the caller and passed by pointer to operators.

---

## Coverage Gaps / TODO

- `LocalInterfaces/` of PlaneBasedOperators partially documented (signatures only from headers; impl details not verified).
- `PlaneBasedGeometry/SharedInterfaces/BasicGeometricPredicates.h` — point-in-polygon, orientation predicates: not yet detailed.
- `PlaneBasedOperators/SharedInterfaces/GeometricProperties.h` — area/volume/mass properties interface: not yet detailed.
- `PlaneBasedOperators/SharedInterfaces/TopoHealer.h` — topology healing: not yet detailed.
- `PlaneBasedOperators/SharedInterfaces/PolyhedronTopologyUpdater.h` — incremental topology update: not yet detailed.
- `Mathematics/SharedInterfaces/PredicatesEngine.h` — predicate dispatch engine: not yet detailed.
- `PerformanceTests/src/TestsPerformance.cpp` — only the shared helper layer is documented here; individual `TestPerformance::*` benchmark flows remain to be expanded if that harness changes again.
- `Infrastructure/SharedInterfaces/Attribute.h` / `Color.h` — attribute system: not yet detailed.
- `PlaneBasedGeometry/SharedInterfaces/Shapes.h` — primitive builders are now partially documented; if this module changes again, expand the remaining polygon-builder details instead of recreating earlier orientation bugs.
