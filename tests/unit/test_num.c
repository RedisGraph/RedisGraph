/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/num.h"

#include "acutest.h"

// There is no need in parametrisation as the code is the same
// for all the other (wider) types.
void test_NumOverflow_CheckedAddU8(void) {
    uint8_t out = 0;
    bool is_ok = checked_add_u8(254, 1, &out);
    TEST_ASSERT(out == 255);
    TEST_ASSERT(is_ok == true);
    is_ok = checked_add_u8(254, 2, &out);
    // The value isn't changed when it is not ok.
    TEST_ASSERT(out == 0);
    TEST_ASSERT(is_ok == false);
    is_ok = checked_add_u8(254, 100, &out);
    // The value isn't changed when it is not ok.
    TEST_ASSERT(out == 98);
    TEST_ASSERT(is_ok == false);
}

TEST_LIST = {
    {"NumOverflow_CheckedAddU8", test_NumOverflow_CheckedAddU8},
    {NULL, NULL}
};
