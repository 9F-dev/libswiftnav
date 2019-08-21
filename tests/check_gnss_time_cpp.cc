#include <check.h>

#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include "check_suites.h"
#include "common/check_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

START_TEST(test_equality) {
  struct testcase {
    gps_time_t a, b;
    bool c;
  } testcases[] = {
      {.a = {567890, 1234}, .b = {567890, 1234}, .c = true},
      {.a = {567890, 1234}, .b = {567890, 1235}, .c = false},
      {.a = {567890, 1234}, .b = {567891, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567891, 1235}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567890, WN_UNKNOWN}, .c = true},
      {.a = {567890, WN_UNKNOWN}, .b = {567891, WN_UNKNOWN}, .c = false},
      {.a = {567890, 1234}, .b = {567890 + 1e-13, 1234}, .c = true},
      {.a = {567890, 1234}, .b = {567890 + 1e-13, 1235}, .c = false},
      {.a = {567890, 1234}, .b = {567891 + 1e-13, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567891 + 1e-13, 1235}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567890 + 1e-13, WN_UNKNOWN}, .c = true},
      {.a = {567890, WN_UNKNOWN}, .b = {567891 + 1e-13, WN_UNKNOWN}, .c = false},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    bool c = testcases[i].a == testcases[i].b;
    fail_unless(c == testcases[i].c,
                "equality test case %d failed, c = %d",
                i,
                c);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    bool c = testcases[i].a != testcases[i].b;
    fail_unless(c == !testcases[i].c,
                "inequality test case %d failed, c = %d",
                i,
                c);
  }
}
END_TEST

START_TEST(test_greater_then) {
  struct testcase {
    gps_time_t a, b;
    bool c;
  } testcases[] = {
      {.a = {567890, 1234}, .b = {567890, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567890, 1235}, .c = false},
      {.a = {567890, 1235}, .b = {567890, 1234}, .c = true},
      {.a = {567890, 1234}, .b = {567891, 1234}, .c = false},
      {.a = {567891, 1234}, .b = {567890, 1234}, .c = true},
      {.a = {567891, 1235}, .b = {567890, 1234}, .c = true},
      {.a = {567890, 1234}, .b = {567891, 1235}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567890, WN_UNKNOWN}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567891, WN_UNKNOWN}, .c = false},
      {.a = {567891, WN_UNKNOWN}, .b = {567890, WN_UNKNOWN}, .c = true},
      {.a = {567890, 1234}, .b = {567890 + 1e-13, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567890 + 1e-13, 1235}, .c = false},
      {.a = {567890, 1235}, .b = {567890 + 1e-13, 1234}, .c = true},
      {.a = {567890, 1234}, .b = {567891 + 1e-13, 1234}, .c = false},
      {.a = {567891, 1234}, .b = {567890 + 1e-13, 1234}, .c = true},
      {.a = {567891, 1235}, .b = {567890 + 1e-13, 1234}, .c = true},
      {.a = {567890, 1234}, .b = {567891 + 1e-13, 1235}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567890 + 1e-13, WN_UNKNOWN}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567891 + 1e-13, WN_UNKNOWN}, .c = false},
      {.a = {567891, WN_UNKNOWN}, .b = {567890 + 1e-13, WN_UNKNOWN}, .c = true},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    bool c = testcases[i].a > testcases[i].b;
    fail_unless(c == testcases[i].c,
                "greater then test case %d failed, c = %d",
                i,
                c);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    bool c = testcases[i].a <= testcases[i].b;
    fail_unless(c == !testcases[i].c,
                "less then or equal to test case %d failed, c = %d",
                i,
                c);
  }
}
END_TEST

START_TEST(test_less_then) {
  struct testcase {
    gps_time_t a, b;
    bool c;
  } testcases[] = {
      {.a = {567890, 1234}, .b = {567890, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567890, 1235}, .c = true},
      {.a = {567890, 1235}, .b = {567890, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567891, 1234}, .c = true},
      {.a = {567891, 1234}, .b = {567890, 1234}, .c = false},
      {.a = {567891, 1235}, .b = {567890, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567891, 1235}, .c = true},
      {.a = {567890, WN_UNKNOWN}, .b = {567890, WN_UNKNOWN}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567891, WN_UNKNOWN}, .c = true},
      {.a = {567891, WN_UNKNOWN}, .b = {567890, WN_UNKNOWN}, .c = false},
      {.a = {567890, 1234}, .b = {567890 + 1e-13, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567890 + 1e-13, 1235}, .c = true},
      {.a = {567890, 1235}, .b = {567890 + 1e-13, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567891 + 1e-13, 1234}, .c = true},
      {.a = {567891, 1234}, .b = {567890 + 1e-13, 1234}, .c = false},
      {.a = {567891, 1235}, .b = {567890 + 1e-13, 1234}, .c = false},
      {.a = {567890, 1234}, .b = {567891 + 1e-13, 1235}, .c = true},
      {.a = {567890, WN_UNKNOWN}, .b = {567890 + 1e-13, WN_UNKNOWN}, .c = false},
      {.a = {567890, WN_UNKNOWN}, .b = {567891 + 1e-13, WN_UNKNOWN}, .c = true},
      {.a = {567891, WN_UNKNOWN}, .b = {567890 + 1e-13, WN_UNKNOWN}, .c = false},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    bool c = testcases[i].a < testcases[i].b;
    fail_unless(c == testcases[i].c,
                "less then test case %d failed, c = %d",
                i,
                c);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    bool c = testcases[i].a >= testcases[i].b;
    fail_unless(c == !testcases[i].c,
                "greater then or equal to test case %d failed, c = %d",
                i,
                c);
  }
}
END_TEST

START_TEST(test_subtract_times) {
  struct testcase {
    gps_time_t a, b;
    double c;
  } testcases[] = {
      {.a = {567890, 1234}, .b = {567890, 1234}, .c = 0},
      {.a = {567890, 1234}, .b = {0, 1234}, .c = 567890},
      {.a = {0, 1234}, .b = {567890, 1234}, .c = -567890},
      {.a = {567890, WN_UNKNOWN}, .b = {0, 1234}, .c = -36910},
      {.a = {222222, 2222}, .b = {2222, WN_UNKNOWN}, .c = 220000},
      {.a = {444444, WN_UNKNOWN}, .b = {2222, WN_UNKNOWN}, .c = -162578},
      {.a = {604578, 1000}, .b = {222.222, 1001}, .c = -444.222},
      {.a = {604578, 1001}, .b = {222.222, 1000}, .c = 1209155.778},
      {.a = {567890, 1235}, .b = {567890, 1234}, .c = 604800},
      {.a = {567890, 1234}, .b = {567890, 1235}, .c = -604800},
  };
  const double tow_tol = 1e-10;
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    double c = testcases[i].a - testcases[i].b;
    fail_unless(fabs(c - testcases[i].c) < tow_tol,
                "subtract times test case %d failed, c = %.12f",
                i,
                c);
  }
}
END_TEST

START_TEST(test_add_duration) {
  struct testcase {
    gps_time_t a;
    double b;
    gps_time_t c;
  } testcases[] = {
      {.a = {567890, 1234}, .b = 0, .c = {567890, 1234}},
      {.a = {567890, 1234}, .b = 10, .c = {567900, 1234}},
      {.a = {567890, 1234}, .b = -10, .c = {567880, 1234}},
      {.a = {567890, 1234}, .b = 604800, .c = {567890, 1235}},
      {.a = {567890, 1235}, .b = -604800, .c = {567890, 1234}},
      {.a = {604795, 1234}, .b = 10, .c = {5, 1235}},
      {.a = {5, 1235}, .b = -10, .c = {604795, 1234}},
      {.a = {567890, WN_UNKNOWN}, .b = 0, .c = {567890, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = 10, .c = {567900, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = -10, .c = {567880, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = 604800, .c = {567890, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = -604800, .c = {567890, WN_UNKNOWN}},
      {.a = {604795, WN_UNKNOWN}, .b = 10, .c = {5, WN_UNKNOWN}},
      {.a = {5, WN_UNKNOWN}, .b = -10, .c = {604795, WN_UNKNOWN}},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t c = testcases[i].a + testcases[i].b;
    fail_unless(c == testcases[i].c,
                "add duration test case %d failed, c.wn = %d, c.tow = %.12f",
                i,
                c.wn,
                c.tow);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t c = testcases[i].a - -testcases[i].b;
    fail_unless(c == testcases[i].c,
                "subtract duration test case %d failed, c.wn = %d, c.tow = %.12f",
                i,
                c.wn,
                c.tow);
  }
}
END_TEST

START_TEST(test_assignment_add_duration) {
  struct testcase {
    gps_time_t a;
    double b;
    gps_time_t c;
  } testcases[] = {
      {.a = {567890, 1234}, .b = 0, .c = {567890, 1234}},
      {.a = {567890, 1234}, .b = 10, .c = {567900, 1234}},
      {.a = {567890, 1234}, .b = -10, .c = {567880, 1234}},
      {.a = {567890, 1234}, .b = 604800, .c = {567890, 1235}},
      {.a = {567890, 1235}, .b = -604800, .c = {567890, 1234}},
      {.a = {604795, 1234}, .b = 10, .c = {5, 1235}},
      {.a = {5, 1235}, .b = -10, .c = {604795, 1234}},
      {.a = {567890, WN_UNKNOWN}, .b = 0, .c = {567890, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = 10, .c = {567900, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = -10, .c = {567880, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = 604800, .c = {567890, WN_UNKNOWN}},
      {.a = {567890, WN_UNKNOWN}, .b = -604800, .c = {567890, WN_UNKNOWN}},
      {.a = {604795, WN_UNKNOWN}, .b = 10, .c = {5, WN_UNKNOWN}},
      {.a = {5, WN_UNKNOWN}, .b = -10, .c = {604795, WN_UNKNOWN}},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t c = testcases[i].a;
    c += testcases[i].b;
    fail_unless(c == testcases[i].c,
                "assignment add duration test case %d failed, c.wn = %d, c.tow = %.12f",
                i,
                c.wn,
                c.tow);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t c = testcases[i].a;
    c -= -testcases[i].b;
    fail_unless(c == testcases[i].c,
                "assignment subtract duration test case %d failed, c.wn = %d, c.tow = %.12f",
                i,
                c.wn,
                c.tow);
  }
}
END_TEST

START_TEST(test_increment_time) {
  struct testcase {
    gps_time_t a;
    gps_time_t b;
  } testcases[] = {
      {.a = {567890, 1234}, .b = {567891, 1234}},
      {.a = {604799, 1234}, .b = {0, 1235}},
      {.a = {567890, WN_UNKNOWN}, .b = {567891, WN_UNKNOWN}},
      {.a = {604799, WN_UNKNOWN}, .b = {0, WN_UNKNOWN}},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t b = testcases[i].a;
    gps_time_t c = ++b;
    fail_unless(b == testcases[i].b && c == testcases[i].b,
                "prefix increment duration test case %d failed, b.wn = %d, b.tow = %.12f, c.wn = %d, c.tow = %.12f",
                i,
                b.wn,
                b.tow,
                c.wn,
                c.tow);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t b = testcases[i].a;
    gps_time_t c = b++;
    fail_unless(b == testcases[i].b && c == testcases[i].a,
                "postfix increment duration test case %d failed, b.wn = %d, b.tow = %.12f, c.wn = %d, c.tow = %.12f",
                i,
                b.wn,
                b.tow,
                c.wn,
                c.tow);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t b = testcases[i].b;
    gps_time_t c = --b;
    fail_unless(b == testcases[i].a && c == testcases[i].a,
                "prefix decrement duration test case %d failed, b.wn = %d, b.tow = %.12f, c.wn = %d, c.tow = %.12f",
                i,
                b.wn,
                b.tow,
                c.wn,
                c.tow);
  }
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct testcase);
       i++) {
    gps_time_t b = testcases[i].b;
    gps_time_t c = b--;
    fail_unless(b == testcases[i].a && c == testcases[i].b,
                "postfix decrement duration test case %d failed, b.wn = %d, b.tow = %.12f, c.wn = %d, c.tow = %.12f",
                i,
                b.wn,
                b.tow,
                c.wn,
                c.tow);
  }
}
END_TEST

Suite *gnss_time_cpp_test_suite(void) {
  Suite *s = suite_create("Time handling (C++)");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_equality);
  tcase_add_test(tc_core, test_greater_then);
  tcase_add_test(tc_core, test_less_then);
  tcase_add_test(tc_core, test_subtract_times);
  tcase_add_test(tc_core, test_add_duration);
  tcase_add_test(tc_core, test_assignment_add_duration);
  tcase_add_test(tc_core, test_increment_time);
  suite_add_tcase(s, tc_core);

  return s;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
