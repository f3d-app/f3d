#ifndef f3d_pseudo_unit_test_h
#define f3d_pseudo_unit_test_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

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
  f3d_test_t* test, const char* label, double actual, double expected)
{
  double tol = 128 * DBL_EPSILON;
  f3d_test_check(test, label, fabs(actual - expected) < tol);
}

// check equality of two integers
static inline void f3d_test_check_int(
  f3d_test_t* test, const char* label, int actual, int expected)
{
  f3d_test_check(test, label, actual == expected);
}

// check equality of two 3-component vectors within a tolerance
static inline void f3d_test_check_vec3(
  f3d_test_t* test, const char* label, const double actual[3], const double expected[3])
{
  double tol = 128 * DBL_EPSILON;
  char full_label[256];

  snprintf(full_label, sizeof(full_label),
    "%s (actual=[%.6g, %.6g, %.6g], expected=[%.6g, %.6g, %.6g])",
    label,
    actual[0], actual[1], actual[2],
    expected[0], expected[1], expected[2]);

  f3d_test_check(test, full_label,
    fabs(actual[0] - expected[0]) < tol &&
    fabs(actual[1] - expected[1]) < tol &&
    fabs(actual[2] - expected[2]) < tol);
}

static inline int f3d_test_result(const f3d_test_t* test)
{
  return test->fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif
