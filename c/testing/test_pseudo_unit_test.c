#include "pseudo_unit_test.h"

int test_pseudo_unit_test()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_test_check(&test, "true condition passes", 1);
  f3d_test_check_int(&test, "int equality passes", 42, 42);
  f3d_test_check_double(&test, "double equality passes", 1.0001, 1.0, 0.001);

  int counts_ok = (test.pass_count == 3 && test.fail_count == 0);
  int result_ok = (f3d_test_result(&test) == EXIT_SUCCESS);

  f3d_test_t inner;
  f3d_test_init(&inner);
  f3d_test_check(&inner, "deliberate failure", 0);
  int fail_detected = (inner.fail_count == 1 && f3d_test_result(&inner) == EXIT_FAILURE);

  f3d_test_t inner2;
  f3d_test_init(&inner2);
  f3d_test_check_int(&inner2, "int inequality fails", 1, 2);
  f3d_test_check_double(&inner2, "double outside tol fails", 1.5, 1.0, 0.1);
  int int_double_fail_detected = (inner2.fail_count == 2 && inner2.pass_count == 0);

  f3d_test_t boundary;
  f3d_test_init(&boundary);
  f3d_test_check_double(&boundary, "exactly at tolerance boundary fails", 1.1, 1.0, 0.1);
  int boundary_ok = (boundary.fail_count == 1);

  return (counts_ok && result_ok && fail_detected && int_double_fail_detected && boundary_ok) ? 0 : 1;
}
