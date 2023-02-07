/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/num.h"

#include "acutest.h"

#define TYPE(width) uint##width ## _t
#define FUNC(width) checked_add_u ## width

#define DEFINE_TEST(name, width) \
void test_NumOverflow_CheckedAddU ##width (void) { \
    TYPE(width) out = 0; \
    static const TYPE(width) MAX = UINT##width##_MAX; \
    bool is_ok = FUNC(width)(MAX - 1, 1, &out); \
    TEST_ASSERT(out == MAX); \
    TEST_ASSERT(is_ok == true); \
    is_ok = FUNC(width)(MAX - 1, 2, &out); \
    /* The value isn't changed when it is not ok. */ \
    TEST_ASSERT(out == 0); \
    TEST_ASSERT(is_ok == false); \
    is_ok = FUNC(width)(MAX - 1, 100, &out); \
    /* The value isn't changed when it is not ok. */ \
    TEST_ASSERT(out == 98); \
    TEST_ASSERT(is_ok == false); \
}

DEFINE_TEST(test_NumOverflow_CheckedAddU8, 8);
DEFINE_TEST(test_NumOverflow_CheckedAddU16, 16);
DEFINE_TEST(test_NumOverflow_CheckedAddU32, 32);
DEFINE_TEST(test_NumOverflow_CheckedAddU64, 64);


TEST_LIST = {
    {"NumOverflow_CheckedAddU8", test_NumOverflow_CheckedAddU8},
    {"NumOverflow_CheckedAddU16", test_NumOverflow_CheckedAddU16},
    {"NumOverflow_CheckedAddU32", test_NumOverflow_CheckedAddU32},
    {"NumOverflow_CheckedAddU64", test_NumOverflow_CheckedAddU64},
    {NULL, NULL}
};
