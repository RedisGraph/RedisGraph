/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/value.h"
#include "../../src/util/rmalloc.h"
#include "../../src/datatypes/set.h"
#include "../../src/datatypes/array.h"
#include "../../src/graph/entities/node.h"
#include "../../src/graph/entities/edge.h"


#ifdef __cplusplus
}
#endif

class ValueTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(ValueTest, TestNumerics) {
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

	SIValue_Free(v);
}

TEST_F(ValueTest, TestStrings) {
	Alloc_Reset();
	SIValue v;
	char const *str = "Test!";
	v = SIValue_FromString(str);
	ASSERT_TRUE(v.type == T_STRING);
	ASSERT_STREQ(v.stringval, "Test!");
	SIValue_Free(v);

	/* Out of double range */
	str = "1.0001e10001";
	v = SIValue_FromString(str);
	ASSERT_TRUE(v.type == T_STRING);
	ASSERT_STREQ(v.stringval, "1.0001e10001");
	SIValue_Free(v);
}

// Idempotence and correctness tests for null, bool, long, double, edge, node, array.
TEST_F(ValueTest, TestNull) {
	SIValue siNull = SI_NullVal();
	SIValue siNullOther = SI_NullVal();
	uint64_t origHashCode = SIValue_HashCode(siNull);
	uint64_t otherHashCode = SIValue_HashCode(siNullOther);
	ASSERT_EQ(origHashCode, otherHashCode);
}

TEST_F(ValueTest, TestHashBool) {
	SIValue siBool = SI_BoolVal(true);
	SIValue siBoolOther = SI_BoolVal(true);
	uint64_t origHashCode = SIValue_HashCode(siBool);
	uint64_t otherHashCode = SIValue_HashCode(siBoolOther);
	ASSERT_EQ(origHashCode, otherHashCode);

	siBool = SI_BoolVal(false);
	origHashCode = SIValue_HashCode(siBool);
	ASSERT_NE(origHashCode, otherHashCode);

	siBoolOther = SI_BoolVal(false);
	otherHashCode = SIValue_HashCode(siBoolOther);
	ASSERT_EQ(origHashCode, otherHashCode);
}

TEST_F(ValueTest, TestHashLong) {
	// INT32 and INT64 tests take too long.
	for(int64_t i = 0; i < 100; i++) {
		int num = rand();
		SIValue siInt64 = SI_LongVal(num);
		SIValue siInt64Other = SI_LongVal(num);
		uint64_t origHashCode = SIValue_HashCode(siInt64);
		uint64_t otherHashCode = SIValue_HashCode(siInt64Other);
		ASSERT_EQ(origHashCode, otherHashCode);
	}
}

TEST_F(ValueTest, TestHashDouble) {
	SIValue siDouble = SI_DoubleVal(3.14);
	SIValue siDoubleOther = SI_DoubleVal(3.14);
	uint64_t origHashCode = SIValue_HashCode(siDouble);
	uint64_t otherHashCode = SIValue_HashCode(siDoubleOther);
	ASSERT_EQ(origHashCode, otherHashCode);

	siDouble = SI_DoubleVal(3.14159265);
	origHashCode = SIValue_HashCode(siDouble);
	ASSERT_NE(origHashCode, otherHashCode);

	siDoubleOther = SI_DoubleVal(3.14159265);
	otherHashCode = SIValue_HashCode(siDoubleOther);
	ASSERT_EQ(origHashCode, otherHashCode);
}

TEST_F(ValueTest, TestEdge) {
	Entity entity;
	entity.id = 0;

	Edge e0;
	e0.entity = &entity;
	SIValue edge = SI_Edge(&e0);

	Edge e0Other;
	e0Other.entity = &entity;
	SIValue edgeOther = SI_Edge(&e0Other);

	// Validate same hashCode for the same value, different address.
	uint64_t origHashCode = SIValue_HashCode(edge);
	uint64_t otherHashCode = SIValue_HashCode(edgeOther);
	ASSERT_EQ(origHashCode, otherHashCode);

	// Change entity id.
	Edge e1;
	entity.id = 1;
	e1.entity = &entity;
	edgeOther = SI_Edge(&e1);
	otherHashCode = SIValue_HashCode(edgeOther);
	ASSERT_NE(origHashCode, otherHashCode);
}

TEST_F(ValueTest, TestNode) {
	Entity entity;
	entity.id = 0;

	Node n0;
	n0.entity = &entity;
	SIValue node = SI_Node(&n0);

	Node n0Other;
	n0Other.entity = &entity;
	SIValue nodeOther = SI_Node(&n0Other);

	// Validate same hashCode for the same value, different address.
	uint64_t origHashCode = SIValue_HashCode(node);
	uint64_t otherHashCode = SIValue_HashCode(nodeOther);
	ASSERT_EQ(origHashCode, otherHashCode);

	// Change entity id.
	entity.id = 1;
	Node n1;
	n1.entity = & entity;
	nodeOther = SI_Node(&n1);
	otherHashCode = SIValue_HashCode(nodeOther);
	ASSERT_NE(origHashCode, otherHashCode);
}

TEST_F(ValueTest, TestArray) {
	SIValue arr = SI_EmptyArray();
	SIValue arrOther = SI_EmptyArray();

	// Test for empty arrays.
	uint64_t origHashCode = SIValue_HashCode(arr);
	uint64_t otherHashCode = SIValue_HashCode(arrOther);
	ASSERT_EQ(origHashCode, otherHashCode);

	SIArray_Append(&arr, SI_LongVal(1));
	origHashCode = SIValue_HashCode(arr);
	ASSERT_NE(origHashCode, otherHashCode);

	SIArray_Append(&arrOther, SI_LongVal(1));
	otherHashCode = SIValue_HashCode(arrOther);
	ASSERT_EQ(origHashCode, otherHashCode);

	SIValue nestedArr = SI_EmptyArray();
	SIArray_Append(&nestedArr, SI_LongVal(2));
	SIArray_Append(&nestedArr, SI_DoubleVal(3.14));

	SIArray_Append(&arr, nestedArr);
	origHashCode = SIValue_HashCode(arr);
	ASSERT_NE(origHashCode, otherHashCode);

	SIArray_Append(&arrOther, nestedArr);
	otherHashCode = SIValue_HashCode(arrOther);
	ASSERT_EQ(origHashCode, otherHashCode);
}

/* Test for difference in hash code for the same binary representation
 * for different types. The value boolean "true" and the integer value "1"
 * have the same binary representation. Given that, their types are different,
 * so this test makes sure that their hash code is different. */
TEST_F(ValueTest, TestHashLongAndBool) {
	SIValue siInt64 = SI_LongVal(1);
	SIValue siBool = SI_BoolVal(true);
	uint64_t longHashCode = SIValue_HashCode(siInt64);
	uint64_t boolHashCode = SIValue_HashCode(siBool);
	ASSERT_NE(longHashCode, boolHashCode);

	siInt64 = SI_LongVal(0);
	siBool = SI_BoolVal(false);

	longHashCode = SIValue_HashCode(siInt64);
	boolHashCode = SIValue_HashCode(siBool);
	ASSERT_NE(longHashCode, boolHashCode);
}

// Test for the same hash code for same semantic value.
TEST_F(ValueTest, TestHashLongAndDouble) {
	SIValue siInt64 = SI_LongVal(1);
	SIValue siDouble = SI_DoubleVal(1.0);
	uint64_t longHashCode = SIValue_HashCode(siInt64);
	uint64_t boolHashCode = SIValue_HashCode(siDouble);
	ASSERT_EQ(longHashCode, boolHashCode);
}

// Test for entities with same id, different types
TEST_F(ValueTest, TestEdgeAndNode) {
	Entity entity;
	entity.id = 0;

	Edge e0;
	e0.entity = &entity;
	SIValue edge = SI_Edge(&e0);

	Node n0;
	n0.entity = &entity;
	SIValue node = SI_Node(&n0);

	ASSERT_NE(SIValue_HashCode(node), SIValue_HashCode(edge));
}

TEST_F(ValueTest, TestSet) {
	Alloc_Reset();
	set *set = Set_New();

	// Set should be empty.
	ASSERT_EQ(Set_Size(set), 0);

	// Populate set.
	Node n;
	Edge e;
	Entity entity;
	entity.id = 0;
	n.entity = &entity;
	e.entity = &entity;
	SIValue arr = SI_Array(2);

	Set_Add(set, arr);
	Set_Add(set, SI_Node(&n));
	Set_Add(set, SI_Edge(&e));
	Set_Add(set, SI_NullVal());
	Set_Add(set, SI_LongVal(0));
	Set_Add(set, SI_DoubleVal(1));
	Set_Add(set, SI_BoolVal(true));

	ASSERT_EQ(Set_Size(set), 7);

	// Make sure all items are in set.
	ASSERT_TRUE(Set_Contains(set, arr));
	ASSERT_TRUE(Set_Contains(set, SI_Node(&n)));
	ASSERT_TRUE(Set_Contains(set, SI_Edge(&e)));
	ASSERT_TRUE(Set_Contains(set, SI_NullVal()));
	ASSERT_TRUE(Set_Contains(set, SI_LongVal(0)));
	ASSERT_TRUE(Set_Contains(set, SI_DoubleVal(1)));
	ASSERT_TRUE(Set_Contains(set, SI_BoolVal(true)));

	// Test for none existing items.
	ASSERT_FALSE(Set_Contains(set, SI_BoolVal(false)));
	ASSERT_FALSE(Set_Contains(set, SI_LongVal(2)));
	ASSERT_FALSE(Set_Contains(set, SI_DoubleVal(3)));

	// Try to introduce duplicates.
	Set_Add(set, arr);
	Set_Add(set, SI_Node(&n));
	Set_Add(set, SI_Edge(&e));
	Set_Add(set, SI_NullVal());
	Set_Add(set, SI_LongVal(0));
	Set_Add(set, SI_DoubleVal(1));
	Set_Add(set, SI_BoolVal(true));

	// Set item count shouldn't change.
	uint64_t set_size = Set_Size(set);
	ASSERT_EQ(set_size, 7);

	// Remove items from set.
	Set_Remove(set, arr);
	ASSERT_FALSE(Set_Contains(set, arr));
	ASSERT_EQ(Set_Size(set), --set_size);

	Set_Remove(set, SI_Node(&n));
	ASSERT_FALSE(Set_Contains(set, SI_Node(&n)));
	ASSERT_EQ(Set_Size(set), --set_size);

	Set_Remove(set, SI_Edge(&e));
	ASSERT_FALSE(Set_Contains(set, SI_Edge(&e)));
	ASSERT_EQ(Set_Size(set), --set_size);

	Set_Remove(set, SI_NullVal());
	ASSERT_FALSE(Set_Contains(set, SI_NullVal()));
	ASSERT_EQ(Set_Size(set), --set_size);

	Set_Remove(set, SI_LongVal(0));
	ASSERT_FALSE(Set_Contains(set, SI_LongVal(0)));
	ASSERT_EQ(Set_Size(set), --set_size);

	Set_Remove(set, SI_DoubleVal(1));
	ASSERT_FALSE(Set_Contains(set, SI_DoubleVal(1)));
	ASSERT_EQ(Set_Size(set), --set_size);

	Set_Remove(set, SI_BoolVal(true));
	ASSERT_FALSE(Set_Contains(set, SI_BoolVal(true)));
	ASSERT_EQ(Set_Size(set), --set_size);

	ASSERT_EQ(Set_Size(set), 0);
	Set_Free(set);
}

