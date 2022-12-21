/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/num.h"
#ifdef __cplusplus
}
#endif

// There is no need in parametrisation as the code is the same
// for all the other (wider) types.
TEST(NumOverflowTest, TestCheckedAddU8) {
    uint8_t out = 0;
    bool is_ok = checked_add_u8(254, 1, &out);
    ASSERT_EQ(out, 255);
    ASSERT_EQ(is_ok, true);
    is_ok = checked_add_u8(254, 2, &out);
    // The value isn't changed when it is not ok.
    ASSERT_EQ(out, 0);
    ASSERT_EQ(is_ok, false);
    is_ok = checked_add_u8(254, 100, &out);
    // The value isn't changed when it is not ok.
    ASSERT_EQ(out, 98);
    ASSERT_EQ(is_ok, false);
}
