# Review Protocols (Files 3 and 4)

## File 3: Code Review Protocol (`RUN_CODE_REVIEW.md`)

### Template

```markdown
# Code Review Protocol: [Project Name]

## Bootstrap (Read First)

Before reviewing, read these files for context:
1. `quality/QUALITY.md` — Quality constitution and fitness-to-purpose scenarios
2. [Main architectural doc]
3. [Key design decisions doc]
4. [Any other essential context]

## What to Check

### Focus Area 1: [Subsystem/Risk Area Name]

**Where:** [Specific files and functions]
**What:** [Specific things to look for]
**Why:** [What goes wrong if this is incorrect]

[Repeat for 4–6 focus areas, mapped to architecture and risk areas from exploration]

## Guardrails

- **Line numbers are mandatory.** If you cannot cite a specific line, do not include the finding.
- **Read function bodies, not just signatures.**
- **If unsure whether something is a bug or intentional**, flag it as a QUESTION rather than a BUG.
- **Grep before claiming missing.** Search before claiming a feature is absent.
- **Do NOT suggest style changes, refactors, or improvements.** Only flag things that are incorrect or could cause failures.

## Output Format

Save findings to `quality/code_reviews/YYYY-MM-DD-reviewer.md`

For each file reviewed:

### filename.ext
- **Line NNN:** [BUG / QUESTION / INCOMPLETE] Description. Expected vs. actual. Why it matters.

### Summary
- Total findings by severity
- Files with no findings
- Overall assessment: SHIP IT / FIX FIRST / NEEDS DISCUSSION
```

### Phase 2: Regression Tests for Confirmed Bugs

After the code review produces findings, write regression tests that reproduce each BUG finding. This transforms "here are potential bugs" into "here are proven bugs with failing tests."

**Name the test file** `quality/test_regression.*` using the project's language conventions.

**Each test should document its origin:**

```cpp
// C++ (Google Test)
TEST(RegressionTests, NullPointerInProcessorAtLine47) {
    // BUG from 2026-03-26-reviewer.md, line 47
    // Processor dereferences pointer without null check
    // Expected: graceful handling; Actual: crash
    auto result = Processor::process(nullptr);
    EXPECT_FALSE(result.has_value());  // Should handle gracefully
}
```

**Report results as a confirmation table:**

```
| Finding | Test | Result | Confirmed? |
|---------|------|--------|------------|
| Null deref in Processor | test_null_deref_processor | FAILED (expected) | YES — bug confirmed |
| Memory leak in Init | test_memory_leak_init | PASSED (unexpected) | NO — needs investigation |
```

---

## File 4: Integration Test Protocol (`RUN_INTEGRATION_TESTS.md`)

### Template

```markdown
# Integration Test Protocol: [Project Name]

## Working Directory

All commands use **relative paths from the project root.** Run everything from the directory containing the project root. Do not `cd` to absolute paths — use `./`, `./build/`, etc.

## Safety Constraints

- DO NOT modify source code
- DO NOT delete files
- ONLY create files in the test results directory
- If something fails, record it and move on

## Pre-Flight Check

Before running integration tests, verify:
- [ ] [Dependencies built — specific command]
- [ ] [Test binaries exist — specific paths]
- [ ] [Test data exists — specific paths]
- [ ] [Clean state — specific cleanup if needed]

## Test Matrix

| Check | Method | Pass Criteria |
|-------|--------|---------------|
| [Happy path flow] | [Specific command or test] | [Specific expected result] |
| [Edge case A] | [Command] | [Expected result] |
| [Performance check] | [Command] | [Expected result] |

## Execution UX (How to Present When Running This Protocol)

### Phase 1: The Plan

Before running anything, show:
```
## Integration Test Plan

**Tests to run:**

| # | Test | What It Checks | Est. Time |
|---|------|---------------|-----------|
| 1 | [Test name] | [One-line description] | ~Xs |
```

### Phase 2: Progress

```
✓ Test 1: [Name] — PASS (0.3s)
✗ Test 2: [Name] — FAIL (assertion failed at line 47)
⧗ Test 3: [Name]... running
```

### Phase 3: Results

```
## Results

| # | Test | Result | Time | Notes |
|---|------|--------|------|-------|
| 1 | [Name] | ✓ PASS | 0.3s | |

**Passed:** 7/8 | **Failed:** 1/8

**Recommendation:** FIX FIRST — [reason]
```

## Reporting

Save results to `quality/results/YYYY-MM-DD-integration.md`
```

### Tips for Writing Good Integration Checks

- Test real end-to-end flows, not just single functions
- Pass criteria must be specific and verifiable
- Include timing expectations for batch/pipeline tests
- Derive quality gates from code — read validation rules, schema enums, and generation logic

### The Field Reference Table (Required Before Writing Quality Gates)

**Why this exists:** AI models confidently write wrong field names even after reading schemas. `document_id` becomes `doc_id`, `float 0-1` becomes `int 0-100`.

**The fix:** Build a Field Reference Table by re-reading each schema file immediately before writing each quality gate. Copy field names character-for-character.

```
## Field Reference Table (built from schemas, not memory)

### Output Schema: MyStructName
Source: path/to/header/or/schema
| Field | Type | Constraints |
|-------|------|-------------|
| field_name | type | constraints |
...
```

**After writing, count fields:** if your quality gates mention a field not in the table, you hallucinated it.
