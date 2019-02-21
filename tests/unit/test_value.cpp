/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/value.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

TEST(ValueTest, TestNumerics) {
    Alloc_Reset();
    SIValue v;
    char const *str = "12345";
    v = SIValue_FromString(str);
    ASSERT_TRUE(v.type == T_DOUBLE);
    ASSERT_EQ(v.doubleval, 12345);

    str = "3.14";
    v = SIValue_FromString(str);
    ASSERT_TRUE(v.type == T_DOUBLE);

    /* Almost equals. */
    ASSERT_LT(v.doubleval - 3.14, 0.0001);

    str = "-9876";
    v = SIValue_FromString(str);
    ASSERT_TRUE(v.type == T_DOUBLE);
    ASSERT_EQ(v.doubleval, -9876);

    str = "+1.0E1";
    v = SIValue_FromString(str);
    ASSERT_TRUE(v.type == T_DOUBLE);
    ASSERT_EQ(v.doubleval, 10);

    SIValue_Free(&v);
}

TEST(ValueTest, TestStrings) {
    Alloc_Reset();
    SIValue v;
    char const *str = "Test!";
    v = SIValue_FromString(str);
    ASSERT_TRUE(v.type == T_STRING);
    ASSERT_STREQ(v.stringval, "Test!");
    SIValue_Free(&v);

    /* Out of double range */
    str = "1.0001e10001";
    v = SIValue_FromString(str);
    ASSERT_TRUE(v.type == T_STRING);
    ASSERT_STREQ(v.stringval, "1.0001e10001");
    SIValue_Free(&v);
}

