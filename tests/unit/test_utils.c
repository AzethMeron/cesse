#include "framework/test.h"
#include "cesse/utils.h"
#include <string.h>

static void test_all_known_codes_have_distinct_strings(void) {
        cesse_error_t codes[] = {
                CESSE_OK, CESSE_ERR_ALLOC, CESSE_ERR_NULLARG, CESSE_ERR_OUT_OF_BOUNDS,
                CESSE_ERR_EMPTY, CESSE_ERR_BAD_ARG, CESSE_ERR_OVERFLOW, CESSE_ERR_UNDERFLOW,
                CESSE_ERR_KEY_NOT_FOUND,
        };
        size_t n = sizeof codes / sizeof codes[0];
        for (size_t i = 0; i < n; i++) {
                ErrorCode code = (ErrorCode)codes[i];
                const char* s = error_code_to_cstring(&code);
                ASSERT_NOT_NULL(s);
                ASSERT_TRUE(s[0] != '\0');
        }
}

static void test_ok_string_differs_from_error_strings(void) {
        ErrorCode ok = CESSE_OK;
        ErrorCode err = CESSE_ERR_ALLOC;
        const char* ok_str = error_code_to_cstring(&ok);
        const char* err_str = error_code_to_cstring(&err);
        ASSERT_TRUE(strcmp(ok_str, err_str) != 0);
}

static void test_null_pointer_handled_without_crash(void) {
        const char* s = error_code_to_cstring(NULL);
        ASSERT_NOT_NULL(s);
}

static void test_unrecognized_code_handled(void) {
        ErrorCode bogus = (ErrorCode)9999;
        const char* s = error_code_to_cstring(&bogus);
        ASSERT_NOT_NULL(s);
}

int main(void) {
        TEST_INIT();
        RUN(test_all_known_codes_have_distinct_strings);
        RUN(test_ok_string_differs_from_error_strings);
        RUN(test_null_pointer_handled_without_crash);
        RUN(test_unrecognized_code_handled);
        return TEST_REPORT();
}
