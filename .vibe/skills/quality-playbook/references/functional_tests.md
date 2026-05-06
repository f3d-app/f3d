# Writing Functional Tests

This is the most important deliverable. Name it using the project's conventions: `test_functional.py` (Python/pytest), `FunctionalSpec.scala` (Scala/ScalaTest), `FunctionalTest.java` (Java/JUnit), `functional.test.ts` (TypeScript/Jest), `functional_test.go` (Go), etc.

## Structure: Three Test Groups

Organize tests into three logical groups:

```
Spec Requirements
    — One test per testable spec section
    — Each test's documentation cites the spec requirement

Fitness Scenarios
    — One test per QUALITY.md scenario (1:1 mapping)
    — Named to match: test_scenario_N_memorable_name

Boundaries and Edge Cases
    — One test per defensive pattern from Step 5
    — Targets null guards, try/catch, normalization, fallbacks
```

## Test Count Heuristic

**Target = (testable spec sections) + (QUALITY.md scenarios) + (defensive patterns from Step 5)**

For a medium-sized project (5–15 source files): typically 35–50 tests.

## Import Pattern: Match the Existing Tests

Before writing any test code, read 2–3 existing test files and identify how they import project modules. Getting this wrong means every test fails with resolution errors.

## Create Test Setup BEFORE Writing Tests

Every fixture or test helper referenced must be defined. If a test depends on shared setup that doesn't exist, it will error during setup.

**Preferred approach:** Write tests that create their own data inline to eliminate cross-file dependencies.

```cpp
// C++ (Google Test) — inline data creation
TEST(SpecRequirements, SectionNRequirement) {
    // [Req: formal — Spec §N] X should produce Y
    auto fixture = MyObject{/* ... */};
    auto result = process(fixture);
    EXPECT_EQ(result.property, expected_value);
}
```

## No Placeholder Tests

Every test must import and call actual project code. If a test body is empty or its only assertion is trivial, delete it. A test that doesn't exercise project code inflates the count and creates false confidence.

## Read Before You Write: The Function Call Map

For every function you plan to test:
1. Read the actual function/method signature
2. Read the documentation (docstrings, comments)
3. Read one existing test that calls it
4. Read real data files to understand data shapes

## Writing Spec-Derived Tests

```cpp
// C++ (Google Test)
class SpecRequirementsTest : public ::testing::Test {
protected:
    // [Req: formal — Spec §N] X should produce Y
};

TEST_F(SpecRequirementsTest, RequirementFromSpecSectionN) {
    // [Req: formal — Spec §N] X should produce Y
    auto result = process(fixture_);
    EXPECT_EQ(result.property, expected_value);
}
```

## Cross-Variant Testing Strategy

If the project handles multiple input types, aim for roughly 30% of tests exercising all variants.

```cpp
// C++ (Google Test) parametrized
class CrossVariantTest : public ::testing::TestWithParam<Variant> {};

TEST_P(CrossVariantTest, FeatureWorksForAllVariants) {
    const auto& variant = GetParam();
    auto result = process(variant.input);
    EXPECT_TRUE(result.has_expected_property());
}

INSTANTIATE_TEST_SUITE_P(AllVariants, CrossVariantTest,
    ::testing::Values(variant_a, variant_b, variant_c));
```

## Anti-Patterns to Avoid

- **Existence-only checks** — Assert the actual value, not just presence
- **Single-variant testing** — Test all input types
- **Positive-only testing** — Test that invalid input does NOT produce bad output
- **Catching exceptions instead of checking output** — Test the outcome

## Fitness-to-Purpose Scenario Tests (1:1 mapping)

```cpp
// C++ (Google Test)
TEST_F(FitnessScenariosTest, Scenario1MemorableName) {
    // [Req: formal — QUALITY.md Scenario 1]
    // Requirement: [What the code must do]
    auto result = process(fixture_);
    EXPECT_TRUE(condition_that_prevents_the_failure(result));
}
```

## Boundary and Negative Tests

```cpp
// C++ (Google Test)
TEST_F(BoundaryTests, FunctionNameGuardsAgainstNullPointer) {
    // [Req: inferred — from function_name() null check]
    auto result = process(nullptr);
    EXPECT_FALSE(result.has_value());  // Graceful handling
}

TEST_F(BoundaryTests, FunctionNameHandlesEmptyInput) {
    // [Req: inferred — from empty-check guard]
    auto result = process(std::vector<int>{});
    EXPECT_TRUE(result.is_empty());
}
```

## Testing at the Right Layer

Ask: "What does the *spec* say should happen?" Test the spec, not the implementation mechanism.
