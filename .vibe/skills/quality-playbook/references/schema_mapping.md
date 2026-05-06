# Schema Type Mapping (Step 5b)

If the project has a type system or validation layer, you need to understand what each field accepts before writing boundary tests. Without this mapping, you'll write mutations that fail at the type level before reaching the code you're trying to test.

## Why This Matters

Consider this common mistake:

```cpp
// C++ — WRONG: tests the assertion, not the requirement
TEST(BoundaryTests, BadValueRejected) {
    // Passing null to a non-nullable parameter triggers an assertion!
    auto result = process(nullptr);  // Crash via assert() or UB
    // Tells you nothing about graceful error handling
}

// C++ — RIGHT: tests the requirement using a schema-valid mutation
TEST(BoundaryTests, EmptyContainerHandledGracefully) {
    // Empty vector is a valid "missing data" input
    auto result = process(std::vector<MyItem>{});
    EXPECT_TRUE(result.is_empty());  // Graceful handling
}
```

The WRONG test crashes or triggers undefined behavior. The RIGHT test uses a value the type system accepts (empty container, default-constructed object, optional with no value) so the test reaches actual processing logic.

## How to Build the Map

For every field you found a defensive pattern for in Step 5, record:

| Field | C++ Type | Accepts | Rejects |
|-------|----------|---------|---------|
| `name` | `std::string` | any string including `""` | (none — any string is valid) |
| `opt_name` | `std::optional<std::string>` | any string or `std::nullopt` | (none) |
| `ptr_obj` | `std::shared_ptr<MyObj>` | valid shared_ptr, `nullptr` | (pointer semantics only) |
| `items` | `std::vector<Item>` | any vector including `{}` | (none) |
| `count` | `int` | any int including 0 and negative | (none at type level) |

## Idiomatic "Missing/Empty" Values in C++

When writing boundary tests, use these idiomatic values:

| Type | "Missing/Empty" Value |
|------|----------------------|
| `std::optional<T>` | `std::nullopt` |
| `std::shared_ptr<T>` | `nullptr` |
| `std::unique_ptr<T>` | `nullptr` |
| `std::string` | `""` |
| `std::vector<T>` | `{}` |
| `int`, `size_t`, etc. | `0` or `-1` |
| `bool` | `false` |
| `struct/class` | default-constructed |

## Rules for Choosing Mutation Values

1. **Prefer values the type system accepts** — test code behavior, not type enforcement
2. **Use `std::nullopt` for optional fields** rather than constructing invalid states
3. **Use `nullptr` for pointer parameters** only if the function is documented to handle it
4. **Use empty containers** (`{}`) to test empty-input handling
5. **Use boundary integers** (0, -1, INT_MAX) to test numeric edge cases

## For Projects Using Custom Validation

If the project has custom validation (pre-conditions, invariants, contracts via `assert()` or EXPECTS/REQUIRES macros), identify which values are valid vs. invalid at the contract level:

```cpp
// If a function has:
// EXPECTS(ptr != nullptr)
// Then nullptr is NOT a valid boundary test value — it violates the contract

// Instead test: what happens with a valid but "empty" object?
auto empty_obj = std::make_shared<MyObj>();  // Valid pointer, empty state
auto result = process(empty_obj);
EXPECT_FALSE(result.is_valid());
```

## When to Skip This Step

If the project has no validation layer (functions directly use their parameters without type-level guards), use any mutation value. But check first — most C++ projects have some combination of null checks, assertions, or contract libraries.
