# Verification Checklist (Phase 3)

Before declaring the quality playbook complete, check every benchmark below. If any fails, go back and fix it.

## Self-Check Benchmarks

### 1. Test Count

Calculate the heuristic target: (testable spec sections) + (QUALITY.md scenarios) + (defensive patterns from Step 5).

- **Well below target** → Likely missed spec requirements or skimmed defensive patterns
- **Near target** → Review whether you tested negative cases and boundaries
- **Above target** → Fine, as long as every test is meaningful

### 2. Scenario Coverage

Count the scenarios in QUALITY.md. Count the scenario test functions in your functional test file. The numbers must match exactly.

### 3. Cross-Variant Coverage

If the project handles N input variants, what percentage of tests exercise all N?

**Heuristic: ~30%.** Common candidates for parametrization: structural completeness, identity verification, required field presence, data relationships, semantic correctness.

### 4. Boundary and Negative Test Count

Count the defensive patterns from Step 5. Count your boundary/negative tests. The ratio should be close to 1:1.

### 5. Assertion Depth

If more than half are presence-only checks, strengthen them to check actual values.

### 6. Layer Correctness

For each test, ask: "Am I testing the *requirement* or the *mechanism*?" Test the outcome, not the implementation.

### 7. Mutation Validity

For every test that mutates a fixture, verify the mutation value is in the "Accepts" column of your Step 5b schema map.

### 8. All Tests Pass — Zero Failures AND Zero Errors

Run the test suite using the project's test runner:

- **C++ (CMake/ctest):** `ctest -R functional --output-on-failure`
- **C++ (Google Test direct):** `./bin/PB2Test --run_test=FunctionalTests --logger=HRF`
- **Python:** `pytest quality/test_functional.py -v`
- **TypeScript:** `npx jest functional.test.ts --verbose`
- **Go:** `go test -v ./quality/...`
- **Rust:** `cargo test`

**Check for both failures AND errors.** Setup errors (fixture not found, import failures) count as broken tests. Both types indicate broken tests.

### 9. Existing Tests Unbroken

Run the project's full test suite (not just your new tests). Your new files should not break anything.

## Documentation Verification

### 10. QUALITY.md Scenarios Reference Real Code

Every scenario should mention actual function names, file names, or patterns that exist in the codebase. Grep for each reference to confirm it exists.

### 11. RUN_CODE_REVIEW.md Is Self-Contained

An AI with no prior context should be able to read it and perform a useful review.

### 12. RUN_INTEGRATION_TESTS.md Is Executable and Field-Accurate

Every command should work. Every check must have a concrete pass/fail criterion.

**Verify quality gates were written from a Field Reference Table, not from memory:**

1. A Field Reference Table exists with a row for every field in every schema
2. Field count matches the actual schema files
3. Field names are copied character-for-character from the files

### 13. RUN_SPEC_AUDIT.md Prompt Is Copy-Pasteable

The definitive audit prompt should work when pasted into Claude Code, Cursor, and Copilot without modification (except file reference syntax).

## Quick Checklist Format

- [ ] Test count near heuristic target (spec sections + scenarios + defensive patterns)
- [ ] Scenario test count matches QUALITY.md scenario count
- [ ] Cross-variant tests ~30% of total
- [ ] Boundary tests approximately equal to defensive pattern count
- [ ] Majority of assertions check values, not just presence
- [ ] All tests assert outcomes, not mechanisms
- [ ] All mutations use schema-valid values
- [ ] All new tests pass (zero failures AND zero errors)
- [ ] All existing tests still pass
- [ ] QUALITY.md scenarios reference real code
- [ ] Code review protocol is self-contained
- [ ] Integration test quality gates were written from a Field Reference Table
- [ ] Spec audit prompt is copy-pasteable
