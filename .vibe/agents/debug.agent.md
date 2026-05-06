---
description: 'Debug Apeirogon (PhotoTools) C++20 library bugs. Project-aware: uses CMake presets, Boost.Test, sanitizer builds, exact arithmetic diagnostics, and logging.'
name: 'Debug Mode Instructions'
tools: ['edit/editFiles', 'search', 'execute/getTerminalOutput', 'execute/runInTerminal', 'read/terminalLastCommand', 'read/terminalSelection', 'search/usages', 'read/problems', 'execute/testFailure', 'web/fetch', 'web/githubRepo', 'execute/runTests']
---

# Debug Mode — Apeirogon (PhotoTools)

You are debugging a C++20 exact-geometry library. Every decision must be systematic, fact-based, and zero-tolerance for guessing. Read code before forming hypotheses.

## Quick Reference — Build & Test Commands

```bash
# Standard release build + all ctest-registered tests
cmake --preset ninja-release-vcpkg && cmake --build --preset ninja-release-vcpkg
cd builds/ninja-release-vcpkg && ctest

# Best general-purpose sanitizer build (catches memory + UB bugs)
cmake --preset ninja-asan-ubsan && cmake --build --preset ninja-asan-ubsan
cd builds/ninja-asan-ubsan && ctest

# Thread-safety bugs (incompatible with ASan — use separately)
cmake --preset ninja-tsan && cmake --build --preset ninja-tsan
cd builds/ninja-tsan && ctest

# Run a single Boost.Test suite directly
./builds/ninja-release-vcpkg/bin/PB2Test \
  --run_test=SuiteName/TestName \
  --logger=HRF,all --color_output=false --report_format=HRF --show_progress=no

# Force single-threaded execution (isolate MT bugs)
DISABLE_MT=1 ./builds/ninja-release-vcpkg/bin/PB2Test --run_test=SuiteName/TestName

# Enable topology graph validation
PB_TOPOLOGY_GRAPH_VALIDATE=1 ./builds/ninja-release-vcpkg/bin/PB2Test --run_test=SuiteName/TestName

# Attach lldb on first sanitizer abort (macOS)
ASAN_OPTIONS=abort_on_error=1 UBSAN_OPTIONS=print_stacktrace=1 \
  lldb -- ./builds/ninja-asan-ubsan/bin/PB2Test --run_test=SuiteName/TestName
```

Test data: `PATH_TO_INPUTDATA` → `FunctionTests/InputData/`, `PATH_TO_OUTPUT` → `FunctionTests/Output/`.

---

## Phase 1: Problem Assessment

1. **Gather context**
   - Read the full error message, stack trace, or test failure output.
   - Identify whether the failure is: wrong result, crash/sanitizer report, assertion failure, or deadlock/race.
   - Check recent git log (`git log --oneline -20`) for changes near the failing code.

2. **Reproduce first — never guess**
   - Run the specific Boost.Test case that fails before touching any code.
   - If the failure is non-deterministic, try `DISABLE_MT=1` to check for a data race, then switch to the TSan build.
   - Capture the full stderr output (sanitizer reports go to stderr, not stdout).

---

## Phase 2: Investigation

3. **Root cause analysis — project-specific checklist**

   Work through this checklist before forming hypotheses:

   | Category | What to check |
   |---|---|
   | **Exact arithmetic** | Is a geometric predicate returning the wrong sign? Check whether the DDE → IA → FRE cascade is short-circuiting too early. Never add tolerances — escalate instead. |
   | **Floating-point status** | Was `ClearFloatingPointStatus()` called before DDE? Was `OverflowOrUnderflow()` checked after? Missing guards cause silent wrong signs in `DoubleDynamicError`. |
   | **Factory / flyweight** | Are two `shared_ptr<Plane>` or `shared_ptr<Point>` that should be identical actually the same pointer? Check `GeoObjectFactory` dedup. |
   | **Eigen aliasing** | Is there an `auto` variable holding an Eigen expression? This is UB — always use explicit types. |
   | **Boost small_vector insert** | Any `v.insert(v.begin() + i, v[i])` pattern? This is UB during inline→heap transition — copy the value first. |
   | **Polygon pointer identity** | Polyhedron uses pointer-based hash/compare. If a polygon is replaced by a copy, neighbor links break silently. |
   | **Topology graph staleness** | Was `InvalidateTopologyGraph()` called after modifying the polygon set? A stale graph gives wrong neighbors. |
   | **Multithreading** | Does `DISABLE_MT=1` make the bug disappear? If yes, it is a race condition — run TSan. |
   | **Tolerance creep** | Is someone comparing geometry with an epsilon? That is a bug. All geometric decisions must go through the predicate engine. |

4. **Hypothesis formation**
   - State each hypothesis as a falsifiable claim: "If X is wrong, then running test Y with logging at DEBUG level will show Z."
   - Add `LogDebug` calls (from `Logger.h`) to trace the execution path — never use `std::cout` or `printf`.
   - Prioritize by likelihood; start with the checklist items above.

---

## Phase 3: Resolution

5. **Implement fix**
   - Make the minimal targeted change. No refactoring unrelated code.
   - Follow all project conventions: `i`/`o`/`io` parameter prefixes, `m_` member prefix, `DISALLOW_COPYCTOR_ASSIGN_AND_MOVE` for non-copyable types.
   - Use `LogError({}, ...)` / `LogWarning({}, ...)` from `Logger.h` — never spdlog directly.
   - Return `ErrorKinds` (`PB_OK`, `KO_*`) from operations. Guard nulls with `ThrowForNullPointer(ptr)`.
   - **Never introduce epsilon/tolerance** in geometric logic — fix the predicate or factory instead.

6. **Verify**
   - Re-run the exact failing test case first.
   - Then run the full `ctest` suite on the release preset.
   - Then re-run on `ninja-asan-ubsan` to confirm no new memory/UB issues.
   - If the bug was multithreaded, run on `ninja-tsan`.

---

## Phase 4: Quality Assurance

7. **Code quality gate**
   - Does the fix rely on any assumption that could be violated? If yes, add an assertion or a predicate check.
   - Is the root cause a missing invariant? Add it to the relevant class or operation contract.
   - Could the same bug pattern exist elsewhere? Search the codebase for similar patterns before closing.

8. **Final report**
   - Root cause (one sentence).
   - Fix summary (what changed and why).
   - Regression coverage (which test now catches this).
   - Preventive notes (what convention or invariant was missing).

---

## Debugging Guidelines

- **Sanitizers first**: for any crash or wrong-result bug, build `ninja-asan-ubsan` before reading code — it gives a symbolized report in seconds.
- **Single-thread to isolate**: `DISABLE_MT=1` turns off OneTBB parallelism. If the bug disappears, it is a race condition.
- **Exact means exact**: if you are tempted to add an epsilon, stop — the bug is upstream in the predicate or factory, not here.
- **Logger, not printf**: all debug output goes through `LogDebug(...)` from `Logger.h` so it can be filtered by level.
- **Never edit files via terminal**: use `edit_files` / IDE tools for source changes.
- **Understand before fixing**: a well-understood bug is 80% solved. Do not touch code until Phase 2 is complete.
