#ifndef CESSE_TEST_FRAMEWORK_H
#define CESSE_TEST_FRAMEWORK_H

/* Minimal test framework: no third-party dependency, one executable
 * per test file, each registered as its own CTest test.
 *
 * Usage in a test .c file:
 *
 *     #include "framework/test.h"
 *     #include "cesse/array.h"
 *
 *     static void test_push_then_pop(void) {
 *             ErrorCode err = CESSE_OK;
 *             Array* a = array_new(4, &err);
 *             ASSERT_EQ(err, CESSE_OK);
 *             ...
 *     }
 *
 *     int main(void) {
 *             TEST_INIT();
 *             RUN(test_push_then_pop);
 *             RUN(test_something_else);
 *             return TEST_REPORT();
 *     }
 *
 * Every ASSERT_* macro `return`s out of the calling function on
 * failure, so test functions must be `static void foo(void)` -- never
 * put an ASSERT_* in a function that needs to return a value.
 */

#include <stdio.h>
#include <string.h>

static int cesse_test_failed_count = 0;

#define TEST_INIT() do { cesse_test_failed_count = 0; } while (0)

#define RUN(fn) do { \
        printf("  %-56s", #fn); \
        fflush(stdout); \
        int cesse_test_before = cesse_test_failed_count; \
        fn(); \
        printf("%s\n", cesse_test_failed_count == cesse_test_before ? "OK" : "FAILED"); \
} while (0)

#define TEST_REPORT() (cesse_test_failed_count == 0 ? 0 : 1)

#define ASSERT_TRUE(cond) do { \
        if (!(cond)) { \
                printf("\n    %s:%d: ASSERT_TRUE(%s) failed\n", __FILE__, __LINE__, #cond); \
                cesse_test_failed_count++; \
                return; \
        } \
} while (0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(a, b) do { \
        if (!((a) == (b))) { \
                printf("\n    %s:%d: ASSERT_EQ(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
                cesse_test_failed_count++; \
                return; \
        } \
} while (0)

#define ASSERT_NE(a, b) do { \
        if ((a) == (b)) { \
                printf("\n    %s:%d: ASSERT_NE(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
                cesse_test_failed_count++; \
                return; \
        } \
} while (0)

#define ASSERT_NULL(p) ASSERT_TRUE((p) == NULL)
#define ASSERT_NOT_NULL(p) ASSERT_TRUE((p) != NULL)

#define ASSERT_STR_EQ(a, b) do { \
        if (strcmp((a), (b)) != 0) { \
                printf("\n    %s:%d: ASSERT_STR_EQ(%s, %s) failed: \"%s\" != \"%s\"\n", \
                       __FILE__, __LINE__, #a, #b, (a), (b)); \
                cesse_test_failed_count++; \
                return; \
        } \
} while (0)

#define ASSERT_NEAR(a, b, eps) do { \
        double cesse_test_diff = (double)(a) - (double)(b); \
        if (cesse_test_diff < 0) { cesse_test_diff = -cesse_test_diff; } \
        if (cesse_test_diff > (double)(eps)) { \
                printf("\n    %s:%d: ASSERT_NEAR(%s, %s, %s) failed: |%.10g - %.10g| > %g\n", \
                       __FILE__, __LINE__, #a, #b, #eps, (double)(a), (double)(b), (double)(eps)); \
                cesse_test_failed_count++; \
                return; \
        } \
} while (0)

#endif
