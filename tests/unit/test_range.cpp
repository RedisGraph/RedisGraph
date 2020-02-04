/*
* Copyright 2018-2020 Redis Labs OP_Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/rmalloc.h"
#include "../../src/ast/ast_shared.h"
#include "../../src/util/range/string_range.h"
#include "../../src/util/range/numeric_range.h"
#include "../../src/util/range/unsigned_range.h"
#include <math.h>
#ifdef __cplusplus
}
#endif

class RangeTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
	}
};

//------------------------------------------------------------------------------
// Numeric range
//------------------------------------------------------------------------------

TEST_F(RangeTest, NumericRangeNew) {
	NumericRange *r = NumericRange_New();

	ASSERT_TRUE(r->valid);
	ASSERT_EQ(r->max, INFINITY);
	ASSERT_EQ(r->min, -INFINITY);
	ASSERT_FALSE(r->include_max);
	ASSERT_FALSE(r->include_min);

	NumericRange_Free(r);
}

TEST_F(RangeTest, NumericRangeValidation) {
	NumericRange *r = NumericRange_New();

	// X > 5 AND X < 5
	r->max = 5;
	r->min = 5;
	r->include_max = false;
	r->include_min = false;
	ASSERT_FALSE(NumericRange_IsValid(r));

	// X >= 5 AND X < 5
	r->max = 5;
	r->min = 5;
	r->include_max = false;
	r->include_min = true;
	ASSERT_FALSE(NumericRange_IsValid(r));

	// X >= 5 AND X <= 5
	r->max = 5;
	r->min = 5;
	r->include_max = true;
	r->include_min = true;
	ASSERT_TRUE(NumericRange_IsValid(r));

	// X > 5 AND X <= 5
	r->max = 5;
	r->min = 5;
	r->include_max = true;
	r->include_min = false;
	ASSERT_FALSE(NumericRange_IsValid(r));

	// (5, 10)  X > 5 AND x < 10.
	r->max = 10;
	r->min = 5;
	r->include_max = false;
	r->include_min = false;
	ASSERT_TRUE(NumericRange_IsValid(r));

	// (5, 10]  X > 5 AND x <= 10.
	r->max = 10;
	r->min = 5;
	r->include_max = true;
	r->include_min = false;
	ASSERT_TRUE(NumericRange_IsValid(r));

	// [5, 10)  X >= 5 AND x < 10.
	r->max = 10;
	r->min = 5;
	r->include_max = false;
	r->include_min = true;
	ASSERT_TRUE(NumericRange_IsValid(r));

	// [5, 10]  X >= 5 AND x =< 10.
	r->max = 10;
	r->min = 5;
	r->include_max = true;
	r->include_min = true;
	ASSERT_TRUE(NumericRange_IsValid(r));

	NumericRange_Free(r);
}

TEST_F(RangeTest, NumericTightenRange) {
	NumericRange *r = NumericRange_New();

	// X < 100
	NumericRange_TightenRange(r, OP_LT, 100);
	ASSERT_EQ(r->max, 100);
	ASSERT_FALSE(r->include_max);

	// X <= 100
	NumericRange_TightenRange(r, OP_LE, 100);
	ASSERT_EQ(r->max, 100);
	ASSERT_FALSE(r->include_max);

	// X >= 50
	NumericRange_TightenRange(r, OP_GE, 50);
	ASSERT_EQ(r->min, 50);
	ASSERT_TRUE(r->include_min);

	// X > 50
	NumericRange_TightenRange(r, OP_GT, 50);
	ASSERT_EQ(r->min, 50);
	ASSERT_FALSE(r->include_min);

	// 75 <= X >= 75
	NumericRange_TightenRange(r, OP_EQUAL, 75);
	ASSERT_EQ(r->min, 75);
	ASSERT_TRUE(r->include_min);
	ASSERT_EQ(r->max, 75);
	ASSERT_TRUE(r->include_max);

	ASSERT_TRUE(NumericRange_IsValid(r));
	NumericRange_Free(r);
}

TEST_F(RangeTest, NumericContainsValue) {
	NumericRange *r = NumericRange_New();
	// -INF < X < INF
	ASSERT_TRUE(NumericRange_ContainsValue(r, 100));

	// X <= 100
	NumericRange_TightenRange(r, OP_LE, 100);
	ASSERT_FALSE(NumericRange_ContainsValue(r, 101));
	ASSERT_TRUE(NumericRange_ContainsValue(r, 100));
	ASSERT_TRUE(NumericRange_ContainsValue(r, -9999));

	// X > -10
	NumericRange_TightenRange(r, OP_GT, -10);
	ASSERT_FALSE(NumericRange_ContainsValue(r, -10));
	ASSERT_TRUE(NumericRange_ContainsValue(r, -9));

	// X >= 0 AND X <= 0
	NumericRange_TightenRange(r, OP_EQUAL, 0);
	ASSERT_FALSE(NumericRange_ContainsValue(r, 1));
	ASSERT_FALSE(NumericRange_ContainsValue(r, -1));
	ASSERT_TRUE(NumericRange_ContainsValue(r, 0));

	// X > 0
	NumericRange_TightenRange(r, OP_GT, 0);
	ASSERT_FALSE(NumericRange_ContainsValue(r, 0));

	NumericRange_Free(r);
}

//------------------------------------------------------------------------------
// String range
//------------------------------------------------------------------------------

TEST_F(RangeTest, StringRangeNew) {
	StringRange *r = StringRange_New();

	ASSERT_TRUE(r->valid);
	ASSERT_TRUE(r->max == NULL);
	ASSERT_TRUE(r->min == NULL);
	ASSERT_FALSE(r->include_max);
	ASSERT_FALSE(r->include_min);

	StringRange_Free(r);
}

TEST_F(RangeTest, StringRangeValidation) {
	StringRange *r;

	// X > "a" AND X < "a"
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LT, "a");
	StringRange_TightenRange(r, OP_GT, "a");
	ASSERT_FALSE(StringRange_IsValid(r));
	StringRange_Free(r);

	// X >= "a" AND X < "a"
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LT, "a");
	StringRange_TightenRange(r, OP_GE, "a");
	ASSERT_FALSE(StringRange_IsValid(r));
	StringRange_Free(r);

	// X >= "a" AND X <= "a"
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LE, "a");
	StringRange_TightenRange(r, OP_GE, "a");
	ASSERT_TRUE(StringRange_IsValid(r));
	StringRange_Free(r);

	// X > "a" AND X <= "a"
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LE, "a");
	StringRange_TightenRange(r, OP_GT, "a");
	ASSERT_FALSE(StringRange_IsValid(r));
	StringRange_Free(r);

	// ("a", "z")  X > "a" AND x < "z".
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LT, "z");
	StringRange_TightenRange(r, OP_GT, "a");
	ASSERT_TRUE(StringRange_IsValid(r));
	StringRange_Free(r);

	// ("a", "z"]  X > "a" AND x <= "z".
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LE, "z");
	StringRange_TightenRange(r, OP_GT, "a");
	ASSERT_TRUE(StringRange_IsValid(r));
	StringRange_Free(r);

	// ["a", "z")  X >= "a" AND x < "z".
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LT, "z");
	StringRange_TightenRange(r, OP_GE, "a");
	ASSERT_TRUE(StringRange_IsValid(r));
	StringRange_Free(r);

	// ["a", "z"]  X >= "a" AND x =< "z".
	r = StringRange_New();
	StringRange_TightenRange(r, OP_LE, "z");
	StringRange_TightenRange(r, OP_GE, "a");
	ASSERT_TRUE(StringRange_IsValid(r));
	StringRange_Free(r);
}

TEST_F(RangeTest, StringTightenRange) {
	StringRange *r = StringRange_New();

	// X < "z"
	StringRange_TightenRange(r, OP_LT, "z");
	ASSERT_STREQ(r->max, "z");
	ASSERT_FALSE(r->include_max);

	// X <= "z"
	StringRange_TightenRange(r, OP_LE, "z");
	ASSERT_STREQ(r->max, "z");
	ASSERT_FALSE(r->include_max);

	// X >= "a"
	StringRange_TightenRange(r, OP_GE, "a");
	ASSERT_STREQ(r->min, "a");
	ASSERT_TRUE(r->include_min);

	// X > "a"
	StringRange_TightenRange(r, OP_GT, "a");
	ASSERT_STREQ(r->min, "a");
	ASSERT_FALSE(r->include_min);

	// "g" <= X >= "g"
	StringRange_TightenRange(r, OP_EQUAL, "g");
	ASSERT_STREQ(r->min, "g");
	ASSERT_TRUE(r->include_min);
	ASSERT_STREQ(r->max, "g");
	ASSERT_TRUE(r->include_max);

	ASSERT_TRUE(StringRange_IsValid(r));
	StringRange_Free(r);
}

TEST_F(RangeTest, StringContainsValue) {
	StringRange *r = StringRange_New();
	// -INF < X < INF
	ASSERT_TRUE(StringRange_ContainsValue(r, "k"));

	// X <= "y"
	StringRange_TightenRange(r, OP_LE, "y");
	ASSERT_FALSE(StringRange_ContainsValue(r, "z"));
	ASSERT_TRUE(StringRange_ContainsValue(r, "y"));
	ASSERT_TRUE(StringRange_ContainsValue(r, "a"));

	// X > "a"
	StringRange_TightenRange(r, OP_GT, "a");
	ASSERT_FALSE(StringRange_ContainsValue(r, "a"));
	ASSERT_TRUE(StringRange_ContainsValue(r, "b"));

	// X >= "k" AND X <= "k"
	StringRange_TightenRange(r, OP_EQUAL, "k");
	ASSERT_FALSE(StringRange_ContainsValue(r, "l"));
	ASSERT_FALSE(StringRange_ContainsValue(r, "j"));
	ASSERT_TRUE(StringRange_ContainsValue(r, "k"));

	// X > "k"
	StringRange_TightenRange(r, OP_GT, "k");
	ASSERT_FALSE(StringRange_ContainsValue(r, "k"));

	StringRange_Free(r);
}

//------------------------------------------------------------------------------
// Unsigned range
//------------------------------------------------------------------------------

TEST_F(RangeTest, UnsignedRangeNew) {
	UnsignedRange *r = UnsignedRange_New();

	ASSERT_TRUE(r->valid);
	ASSERT_EQ(r->max, UINT64_MAX);
	ASSERT_EQ(r->min, 0);
	ASSERT_TRUE(r->include_max);
	ASSERT_TRUE(r->include_min);

	UnsignedRange_Free(r);
}

TEST_F(RangeTest, UnsignedRangeValidation) {
	UnsignedRange *r = UnsignedRange_New();

	// X > 5 AND X < 5
	r->max = 5;
	r->min = 5;
	r->include_max = false;
	r->include_min = false;
	ASSERT_FALSE(UnsignedRange_IsValid(r));

	// X >= 5 AND X < 5
	r->max = 5;
	r->min = 5;
	r->include_max = false;
	r->include_min = true;
	ASSERT_FALSE(UnsignedRange_IsValid(r));

	// X >= 5 AND X <= 5
	r->max = 5;
	r->min = 5;
	r->include_max = true;
	r->include_min = true;
	ASSERT_TRUE(UnsignedRange_IsValid(r));

	// X > 5 AND X <= 5
	r->max = 5;
	r->min = 5;
	r->include_max = true;
	r->include_min = false;
	ASSERT_FALSE(UnsignedRange_IsValid(r));

	// (5, 10)  X > 5 AND x < 10.
	r->max = 10;
	r->min = 5;
	r->include_max = false;
	r->include_min = false;
	ASSERT_TRUE(UnsignedRange_IsValid(r));

	// (5, 10]  X > 5 AND x <= 10.
	r->max = 10;
	r->min = 5;
	r->include_max = true;
	r->include_min = false;
	ASSERT_TRUE(UnsignedRange_IsValid(r));

	// [5, 10)  X >= 5 AND x < 10.
	r->max = 10;
	r->min = 5;
	r->include_max = false;
	r->include_min = true;
	ASSERT_TRUE(UnsignedRange_IsValid(r));

	// [5, 10]  X >= 5 AND x =< 10.
	r->max = 10;
	r->min = 5;
	r->include_max = true;
	r->include_min = true;
	ASSERT_TRUE(UnsignedRange_IsValid(r));

	UnsignedRange_Free(r);
}

TEST_F(RangeTest, UnsignedTightenRange) {
	UnsignedRange *r = UnsignedRange_New();

	// X < 100
	UnsignedRange_TightenRange(r, OP_LT, 100);
	ASSERT_EQ(r->max, 100);
	ASSERT_FALSE(r->include_max);

	// X <= 100
	UnsignedRange_TightenRange(r, OP_LE, 100);
	ASSERT_EQ(r->max, 100);
	ASSERT_FALSE(r->include_max);

	// X >= 50
	UnsignedRange_TightenRange(r, OP_GE, 50);
	ASSERT_EQ(r->min, 50);
	ASSERT_TRUE(r->include_min);

	// X > 50
	UnsignedRange_TightenRange(r, OP_GT, 50);
	ASSERT_EQ(r->min, 50);
	ASSERT_FALSE(r->include_min);

	// 75 <= X >= 75
	UnsignedRange_TightenRange(r, OP_EQUAL, 75);
	ASSERT_EQ(r->min, 75);
	ASSERT_TRUE(r->include_min);
	ASSERT_EQ(r->max, 75);
	ASSERT_TRUE(r->include_max);

	ASSERT_TRUE(UnsignedRange_IsValid(r));
	UnsignedRange_Free(r);
}

TEST_F(RangeTest, UnsignedContainsValue) {
	UnsignedRange *r = UnsignedRange_New();
	// 0 <= X < UINT64_MAX
	ASSERT_TRUE(UnsignedRange_ContainsValue(r, 100));

	// X <= 100
	UnsignedRange_TightenRange(r, OP_LE, 100);
	ASSERT_FALSE(UnsignedRange_ContainsValue(r, 101));
	ASSERT_TRUE(UnsignedRange_ContainsValue(r, 100));
	ASSERT_TRUE(UnsignedRange_ContainsValue(r, 99));

	// X >= 0 AND X <= 0
	UnsignedRange_TightenRange(r, OP_EQUAL, 0);
	ASSERT_FALSE(UnsignedRange_ContainsValue(r, 1));
	ASSERT_TRUE(UnsignedRange_ContainsValue(r, 0));

	// X > 0
	UnsignedRange_TightenRange(r, OP_GT, 0);
	ASSERT_FALSE(UnsignedRange_ContainsValue(r, 0));

	UnsignedRange_Free(r);
}
