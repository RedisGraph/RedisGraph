/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../src/util/rmalloc.h"
#include "../../src/value.h"
#include "../../src/graph/entities/edge.h"
#include "../../src/graph/entities/node.h"
#include "../../src/datatypes/array.h"

#ifdef __cplusplus
}
#endif

class HashCodeTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};

// Idempotence and correctness tests for null, bool, long, double, edge, node, array.

TEST_F(HashCodeTest, TestNull) {
	SIValue siNull = SI_NullVal();
	SIValue siNullOther = SI_NullVal();
	uint64_t origHashCode = SIValue_HashCode(siNull);
	uint64_t otherHashCode = SIValue_HashCode(siNullOther);
	ASSERT_EQ(origHashCode, otherHashCode);
}

TEST_F(HashCodeTest, TestHashBool) {
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

TEST_F(HashCodeTest, TestHashLong) {
	// INT32 and INT64 tests take too long.
	for(int64_t i = INT16_MIN; i < INT16_MAX; i++) {
		SIValue siInt64 = SI_LongVal(i);
		SIValue siInt64Other = SI_LongVal(i);
		uint64_t origHashCode = SIValue_HashCode(siInt64);
		uint64_t otherHashCode = SIValue_HashCode(siInt64Other);
		ASSERT_EQ(origHashCode, otherHashCode);
	}
}

TEST_F(HashCodeTest, TestHashDouble) {
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


TEST_F(HashCodeTest, TestEdge) {
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

TEST_F(HashCodeTest, TestNode) {
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

TEST_F(HashCodeTest, TestArray) {
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
}

/* Test for difference in hash code for the same binary representation
 * for different types.*/
TEST_F(HashCodeTest, TestHashLongAndBool) {
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
TEST_F(HashCodeTest, TestHashLongAndDouble) {
	SIValue siInt64 = SI_LongVal(1);
	SIValue siDouble = SI_DoubleVal(1.0);
	uint64_t longHashCode = SIValue_HashCode(siInt64);
	uint64_t boolHashCode = SIValue_HashCode(siDouble);
	ASSERT_EQ(longHashCode, boolHashCode);
}

// Test for entities with same id, different types
TEST_F(HashCodeTest, TestEdgeAndNode) {
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
