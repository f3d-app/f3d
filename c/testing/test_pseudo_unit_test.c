#include "pseudo_unit_test.h"

int test_pseudo_unit_test()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_test_check(&test, "true condition passes", 1);
  f3d_test_check_int(&test, "int equality passes", 42, 42);
  f3d_test_check_double(&test, "double equality passes", 1.0001, 1.0, 0.001);

  int counts_ok = (test.pass_count == 3 && test.fail_count == 0);

  f3d_test_t inner;
  f3d_test_init(&inner);
  f3d_test_check(&inner, "deliberate failure", 0);
  int fail_detected = (inner.fail_count == 1 && f3d_test_result(&inner) == EXIT_FAILURE);

  return (counts_ok && fail_detected) ? 0 : 1;
}