#ifndef f3d_pseudo_unit_test_h
#define f3d_pseudo_unit_test_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  int pass_count;
  int fail_count;
} f3d_test_t;

static inline void f3d_test_init(f3d_test_t* test)
{
  test->pass_count = 0;
  test->fail_count = 0;
}

// evaluate a boolean condition
static inline void f3d_test_check(f3d_test_t* test, const char* label, int condition)
{
  if (condition)
  {
    test->pass_count++;
    printf("[PASS] %s\n", label);
  }
  else
  {
    test->fail_count++;
    fprintf(stderr, "[FAIL] %s\n", label);
  }
}

// check equality of two doubles within a tolerance
static inline void f3d_test_check_double(
  f3d_test_t* test, const char* label, double actual, double expected, double tol)
{
  f3d_test_check(test, label, fabs(actual - expected) < tol);
}

// check equality of two integers
static inline void f3d_test_check_int(
  f3d_test_t* test, const char* label, long actual, long expected)
{
  f3d_test_check(test, label, actual == expected);
}

static inline int f3d_test_result(const f3d_test_t* test)
{
  return test->fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif