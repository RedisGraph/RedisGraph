/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./acutest.h"
#include "../../src/value.h"
#include "../../src/util/rmalloc.h"
#include "../../src/datatypes/set.h"
#include "../../src/datatypes/array.h"
#include "../../src/graph/entities/node.h"
#include "../../src/graph/entities/edge.h"

void test_numerics(void) {
	Alloc_Reset();
	SIValue v;
	char const *str = "12345";
	v = SIValue_FromString(str);
	TEST_CHECK(v.type == T_DOUBLE);
	TEST_CHECK(v.doubleval == 12345);

	str = "3.14";
	v = SIValue_FromString(str);
	TEST_CHECK(v.type == T_DOUBLE);

	/* Almost equals. */
	TEST_CHECK(v.doubleval - 3.14 < 0.0001);

	str = "-9876";
	v = SIValue_FromString(str);
	TEST_CHECK(v.type == T_DOUBLE);
	TEST_CHECK(v.doubleval == -9876);

	str = "+1.0E1";
	v = SIValue_FromString(str);
	TEST_CHECK(v.type == T_DOUBLE);
	TEST_CHECK(v.doubleval == 10);

	SIValue_Free(v);
}

void test_strings(void) {
	Alloc_Reset();
	SIValue v;
	char const *str = "Test!";
	v = SIValue_FromString(str);
	TEST_CHECK(v.type == T_STRING);
	TEST_CHECK(!strcmp(v.stringval, "Test!"));
	SIValue_Free(v);

	/* Out of double range */
	str = "1.0001e10001";
	v = SIValue_FromString(str);
	TEST_CHECK(v.type == T_STRING);
	TEST_CHECK(!strcmp(v.stringval, "1.0001e10001"));
	SIValue_Free(v);
}

// Idempotence and correctness tests for null, bool, long, double, edge, node, array.
void test_null(void) {
	Alloc_Reset();
	SIValue siNull = SI_NullVal();
	SIValue siNullOther = SI_NullVal();
	uint64_t origHashCode = SIValue_HashCode(siNull);
	uint64_t otherHashCode = SIValue_HashCode(siNullOther);
	TEST_CHECK(origHashCode == otherHashCode);
}

void test_hash_bool(void) {
	Alloc_Reset();
	SIValue siBool = SI_BoolVal(true);
	SIValue siBoolOther = SI_BoolVal(true);
	uint64_t origHashCode = SIValue_HashCode(siBool);
	uint64_t otherHashCode = SIValue_HashCode(siBoolOther);
	TEST_CHECK(origHashCode == otherHashCode);

	siBool = SI_BoolVal(false);
	origHashCode = SIValue_HashCode(siBool);
	TEST_CHECK(origHashCode != otherHashCode);

	siBoolOther = SI_BoolVal(false);
	otherHashCode = SIValue_HashCode(siBoolOther);
	TEST_CHECK(origHashCode == otherHashCode);
}

void test_hash_long(void) {
	Alloc_Reset();
	// INT32 and INT64 tests take too long.
	for(int64_t i = 0; i < 100; i++) {
		int num = rand();
		SIValue siInt64 = SI_LongVal(num);
		SIValue siInt64Other = SI_LongVal(num);
		uint64_t origHashCode = SIValue_HashCode(siInt64);
		uint64_t otherHashCode = SIValue_HashCode(siInt64Other);
		TEST_CHECK(origHashCode == otherHashCode);
	}
}

void test_hash_double(void) {
	Alloc_Reset();
	SIValue siDouble = SI_DoubleVal(3.14);
	SIValue siDoubleOther = SI_DoubleVal(3.14);
	uint64_t origHashCode = SIValue_HashCode(siDouble);
	uint64_t otherHashCode = SIValue_HashCode(siDoubleOther);
	TEST_CHECK(origHashCode == otherHashCode);

	siDouble = SI_DoubleVal(3.14159265);
	origHashCode = SIValue_HashCode(siDouble);
	TEST_CHECK(origHashCode != otherHashCode);

	siDoubleOther = SI_DoubleVal(3.14159265);
	otherHashCode = SIValue_HashCode(siDoubleOther);
	TEST_CHECK(origHashCode == otherHashCode);
}

void test_edge(void) {
	Alloc_Reset();
	Entity entity;

	Edge e0;
	e0.id = 0;
	e0.entity = &entity;
	SIValue edge = SI_Edge(&e0);

	Edge e0Other = e0;
	SIValue edgeOther = SI_Edge(&e0Other);

	// Validate same hashCode for the same value, different address.
	uint64_t origHashCode = SIValue_HashCode(edge);
	uint64_t otherHashCode = SIValue_HashCode(edgeOther);
	TEST_CHECK(origHashCode == otherHashCode);

	// Change entity id.
	Edge e1 = e0;
	e1.id = 1;
	edgeOther = SI_Edge(&e1);
	otherHashCode = SIValue_HashCode(edgeOther);
	TEST_CHECK(origHashCode != otherHashCode);
}

void test_node(void) {
	Alloc_Reset();
	Entity entity;

	Node n0;
	n0.id = 0;
	n0.entity = &entity;
	SIValue node = SI_Node(&n0);

	Node n0Other = n0;
	SIValue nodeOther = SI_Node(&n0Other);

	// Validate same hashCode for the same value, different address.
	uint64_t origHashCode = SIValue_HashCode(node);
	uint64_t otherHashCode = SIValue_HashCode(nodeOther);
	TEST_CHECK(origHashCode == otherHashCode);

	// Change entity id.
	Node n1 = n0;
	n1.id = 1;
	nodeOther = SI_Node(&n1);
	otherHashCode = SIValue_HashCode(nodeOther);
	TEST_CHECK(origHashCode != otherHashCode);
}

void test_array(void) {
	Alloc_Reset();
	SIValue arr = SI_EmptyArray();
	SIValue arrOther = SI_EmptyArray();

	// Test for empty arrays.
	uint64_t origHashCode = SIValue_HashCode(arr);
	uint64_t otherHashCode = SIValue_HashCode(arrOther);
	TEST_CHECK(origHashCode == otherHashCode);

	SIArray_Append(&arr, SI_LongVal(1));
	origHashCode = SIValue_HashCode(arr);
	TEST_CHECK(origHashCode != otherHashCode);

	SIArray_Append(&arrOther, SI_LongVal(1));
	otherHashCode = SIValue_HashCode(arrOther);
	TEST_CHECK(origHashCode == otherHashCode);

	SIValue nestedArr = SI_EmptyArray();
	SIArray_Append(&nestedArr, SI_LongVal(2));
	SIArray_Append(&nestedArr, SI_DoubleVal(3.14));

	SIArray_Append(&arr, nestedArr);
	origHashCode = SIValue_HashCode(arr);
	TEST_CHECK(origHashCode != otherHashCode);

	SIArray_Append(&arrOther, nestedArr);
	otherHashCode = SIValue_HashCode(arrOther);
	TEST_CHECK(origHashCode == otherHashCode);
}

/* Test for difference in hash code for the same binary representation
 * for different types. The value boolean "true" and the integer value "1"
 * have the same binary representation. Given that, their types are different,
 * so this test makes sure that their hash code is different. */
void test_hash_long_and_bool(void) {
	Alloc_Reset();
	SIValue siInt64 = SI_LongVal(1);
	SIValue siBool = SI_BoolVal(true);
	uint64_t longHashCode = SIValue_HashCode(siInt64);
	uint64_t boolHashCode = SIValue_HashCode(siBool);
	TEST_CHECK(longHashCode != boolHashCode);

	siInt64 = SI_LongVal(0);
	siBool = SI_BoolVal(false);

	longHashCode = SIValue_HashCode(siInt64);
	boolHashCode = SIValue_HashCode(siBool);
	TEST_CHECK(longHashCode != boolHashCode);
}

// Test for the same hash code for same semantic value.
void test_hash_long_and_double(void) {
	Alloc_Reset();
	SIValue siInt64 = SI_LongVal(1);
	SIValue siDouble = SI_DoubleVal(1.0);
	uint64_t longHashCode = SIValue_HashCode(siInt64);
	uint64_t boolHashCode = SIValue_HashCode(siDouble);
	TEST_CHECK(longHashCode == boolHashCode);
}

// Test for entities with same id, different types
void test_edge_and_node(void) {
	Alloc_Reset();
	Entity entity;

	Edge e0;
	e0.id = 0;
	e0.entity = &entity;
	SIValue edge = SI_Edge(&e0);

	Node n0;
	n0.entity = &entity;
	SIValue node = SI_Node(&n0);

	TEST_CHECK(SIValue_HashCode(node) != SIValue_HashCode(edge));
}

void test_set(void) {
	Alloc_Reset();
	set *set = Set_New();

	// Set should be empty.
	TEST_CHECK(Set_Size(set) == 0);

	// Populate set.
	Node n;
	Edge e;
	Entity entity;
	n.id = 0;
	n.entity = &entity;
	e.id = 0;
	e.entity = &entity;
	SIValue arr = SI_Array(2);

	Set_Add(set, arr);
	Set_Add(set, SI_Node(&n));
	Set_Add(set, SI_Edge(&e));
	Set_Add(set, SI_NullVal());
	Set_Add(set, SI_LongVal(0));
	Set_Add(set, SI_DoubleVal(1));
	Set_Add(set, SI_BoolVal(true));

	TEST_CHECK(Set_Size(set) == 7);

	// Make sure all items are in set.
	TEST_CHECK(Set_Contains(set, arr));
	TEST_CHECK(Set_Contains(set, SI_Node(&n)));
	TEST_CHECK(Set_Contains(set, SI_Edge(&e)));
	TEST_CHECK(Set_Contains(set, SI_NullVal()));
	TEST_CHECK(Set_Contains(set, SI_LongVal(0)));
	TEST_CHECK(Set_Contains(set, SI_DoubleVal(1)));
	TEST_CHECK(Set_Contains(set, SI_BoolVal(true)));

	// Test for none existing items.
	TEST_CHECK(!Set_Contains(set, SI_BoolVal(false)));
	TEST_CHECK(!Set_Contains(set, SI_LongVal(2)));
	TEST_CHECK(!Set_Contains(set, SI_DoubleVal(3)));

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
	TEST_CHECK(set_size == 7);

	// Remove items from set.
	Set_Remove(set, arr);
	TEST_CHECK(!Set_Contains(set, arr));
	TEST_CHECK(Set_Size(set) == --set_size);

	Set_Remove(set, SI_Node(&n));
	TEST_CHECK(!Set_Contains(set, SI_Node(&n)));
	TEST_CHECK(Set_Size(set) == --set_size);

	Set_Remove(set, SI_Edge(&e));
	TEST_CHECK(!Set_Contains(set, SI_Edge(&e)));
	TEST_CHECK(Set_Size(set) == --set_size);

	Set_Remove(set, SI_NullVal());
	TEST_CHECK(!Set_Contains(set, SI_NullVal()));
	TEST_CHECK(Set_Size(set) == --set_size);

	Set_Remove(set, SI_LongVal(0));
	TEST_CHECK(!Set_Contains(set, SI_LongVal(0)));
	TEST_CHECK(Set_Size(set) == --set_size);

	Set_Remove(set, SI_DoubleVal(1));
	TEST_CHECK(!Set_Contains(set, SI_DoubleVal(1)));
	TEST_CHECK(Set_Size(set) == --set_size);

	Set_Remove(set, SI_BoolVal(true));
	TEST_CHECK(!Set_Contains(set, SI_BoolVal(true)));
	TEST_CHECK(Set_Size(set) == --set_size);

	TEST_CHECK(Set_Size(set) == 0);
	Set_Free(set);
}

TEST_LIST = {
	{ "test_numerics", test_numerics },
	{ "test_strings", test_strings },
	{ "test_null", test_null },
	{ "test_hash_bool", test_hash_bool },
	{ "test_hash_long", test_hash_long },
	{ "test_hash_double", test_hash_double },
	{ "test_edge", test_edge },
	{ "test_node", test_node },
	{ "test_array", test_array },
	{ "test_hash_long_and_bool", test_hash_long_and_bool },
	{ "test_hash_long_and_double", test_hash_long_and_double },
	{ "test_edge_and_node", test_edge_and_node },
	{ "test_set", test_set },
	{ NULL, NULL }     // zeroed record marking the end of the list
};

