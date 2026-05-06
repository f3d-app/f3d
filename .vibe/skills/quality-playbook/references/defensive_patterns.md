# Finding Defensive Patterns (Step 5)

Defensive code patterns are evidence of past failures or known risks. Every null guard, try/catch, normalization function, and sentinel check exists because something went wrong — or because someone anticipated it would.

## Systematic Search

Grep the codebase methodically for defensive-code indicators:

**Null/nil guards:**

| Language | Grep pattern |
|---|---|
| Python | `None`, `is None`, `is not None` |
| Java | `null`, `Optional`, `Objects.requireNonNull` |
| Scala | `Option`, `None`, `.getOrElse`, `.isEmpty` |
| TypeScript | `undefined`, `null`, `??`, `?.` |
| Go | `== nil`, `!= nil`, `if err != nil` |
| Rust | `Option`, `unwrap`, `.is_none()`, `?` |
| C++ | `nullptr`, `== nullptr`, `!= nullptr`, `assert(` |

**Exception/error handling:**

| Language | Grep pattern |
|---|---|
| Python | `except`, `try:`, `raise` |
| Java/Scala | `catch`, `throws`, `try {` |
| TypeScript | `catch`, `throw`, `.catch(` |
| Go | `if err != nil`, `errors.New`, `fmt.Errorf` |
| Rust | `Result`, `Err(`, `unwrap_or`, `match` |
| C++ | `catch`, `throw`, `try {`, `noexcept` |

**Sentinel values, fallbacks, boundary checks:**

Search for `== 0`, `< 0`, `default`, `fallback`, `else`, `match`, `switch` — language-agnostic.

## What to Look For Beyond Grep

- **Bugs that were fixed** — Git history, TODO comments, workarounds
- **Design decisions** — Comments explaining "why" not just "what"
- **External data quirks** — Any place the code normalizes, validates, or rejects input
- **Parsing functions** — Every parser has failure modes
- **Boundary conditions** — Zero values, empty strings, maximum ranges

## Converting Findings to Scenarios

For each defensive pattern, ask: "What failure does this prevent?"

```markdown
### Scenario N: [Memorable Name]

**Requirement tag:** [Req: inferred — from function_name() behavior]

**What happened:** [The failure mode this code prevents. Reference the actual function and line. Frame as a vulnerability analysis.]

**The requirement:** [What the code must do to prevent this failure.]

**How to verify:** [A concrete test that would fail if this regressed.]
```

## Converting Findings to Boundary Tests

Each defensive pattern maps to a boundary test — use schema-valid mutation values (see schema_mapping.md):

```cpp
// C++ (Google Test)
// [Req: inferred — from function_name() null check]
TEST(BoundaryTests, HandlesNullInput) {
    // Use valid "missing" value — null/nullptr, default-constructed, etc.
    auto result = process(nullptr);
    // Assert graceful handling, not crash
    EXPECT_FALSE(result.has_value());
}
```

## State Machine Patterns

When you find status fields, lifecycle phases, or mode flags, trace the full state machine:

**How to find state machines:**

| Language | Grep pattern |
|---|---|
| Python | `status`, `state`, `phase`, `== "running"`, `== "pending"` |
| C++ | `enum.*State`, `enum.*Status`, `m_state`, `m_status`, `switch.*state` |
| Java | `enum.*Status`, `.getStatus()`, `switch.*status` |
| TypeScript | `status:`, `state:`, `Status =` |

**For each state machine found:**

1. List every possible state value
2. For each handler, verify it handles ALL states
3. Look for states you can enter but never leave
4. Look for operations that should be available in a state but are blocked

## Missing Safeguard Patterns

Search for operations that commit the user to expensive, irreversible, or long-running work without adequate preview or confirmation:

| Pattern | What to look for |
|---|---|
| Pre-commit information gap | Operations that start batch jobs without showing estimated scope |
| No termination condition | Polling loops that never check whether all work is done |
| Retry without backoff | Error handling that retries immediately without exponential backoff |

## Minimum Bar

Find at least 2–3 defensive patterns per source file in the core logic modules. For a medium-sized project (5–15 source files), expect 15–30 defensive patterns total.
