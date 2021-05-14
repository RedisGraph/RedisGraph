/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../src/util/rmalloc.h"
#include "../../src/graph/entities/attribute_set.h"

#ifdef __cplusplus
}
#endif

class AttributeSetTest : public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(AttributeSetTest, EmptySet) {
	AttributeSet set = AttributeSet_New(0);

	//--------------------------------------------------------------------------
	// AttributeSet should be empty
	//--------------------------------------------------------------------------

	ASSERT_EQ(0, AttributeSet_AttributeCount(set));


	//--------------------------------------------------------------------------
	// try getting a value
	//--------------------------------------------------------------------------

	uint idx;
	ASSERT_FALSE(AttributeSet_Contains(set, 0, &idx));
	ASSERT_EQ(ATTRIBUTE_NOTFOUND, AttributeSet_GetAttr(set, 0));
	ASSERT_EQ(ATTRIBUTE_UNKNOWN, idx);

	SIValue val;
	Attribute_ID id;

	AttributeSet_GetAttrIdx(set, 0, &val, &id);

	ASSERT_TRUE(SIValue_IsNull(val));
	ASSERT_EQ(ATTRIBUTE_UNKNOWN, id);

	//--------------------------------------------------------------------------
	// try removing a non-existent value
	//--------------------------------------------------------------------------

	bool removed;
	AttributeSet returned_set = AttributeSet_RemoveAttr(&set, 0, &removed);

	ASSERT_FALSE(removed);
	ASSERT_EQ(set, returned_set); // expect no realloc

	//--------------------------------------------------------------------------
	// clear the AttributeSet
	//--------------------------------------------------------------------------

	uint removed_count;
	returned_set = AttributeSet_Clear(&set, &removed_count);
	ASSERT_FALSE(removed_count);

	// clean up
	AttributeSet_Free(set);
}

TEST_F(AttributeSetTest, AddAttribute) {
	AttributeSet set = AttributeSet_New(0);

	//--------------------------------------------------------------------------
	// add 2 attributes
	//--------------------------------------------------------------------------

	SIValue attr1 = SI_ConstStringVal("attr1");
	SIValue attr2 = SI_LongVal(2);

	set = AttributeSet_SetAttr(&set, 0, attr1);
	ASSERT_EQ(1, AttributeSet_AttributeCount(set));

	set = AttributeSet_SetAttr(&set, 5, attr2);
	ASSERT_EQ(2, AttributeSet_AttributeCount(set));

	//--------------------------------------------------------------------------
	// validate AttributeSet contents
	//--------------------------------------------------------------------------

	uint idx;
	ASSERT_TRUE(AttributeSet_Contains(set, 0, &idx));
	ASSERT_EQ(0, idx);
	SIValue *ret = AttributeSet_GetAttr(set, 0);
	ASSERT_EQ(0, SIValue_Compare(attr1, *ret, NULL));
	Attribute_ID id;
	AttributeSet_GetAttrIdx(set, 0, ret, &id);
	ASSERT_EQ(0, SIValue_Compare(attr1, *ret, NULL));
	ASSERT_EQ(0, id);

	ASSERT_TRUE(AttributeSet_Contains(set, 5, &idx));
	ASSERT_EQ(1, idx);
	ret = AttributeSet_GetAttr(set, 5);
	ASSERT_EQ(0, SIValue_Compare(attr2, *ret, NULL));
	AttributeSet_GetAttrIdx(set, 1, ret, &id);
	ASSERT_EQ(0, SIValue_Compare(attr2, *ret, NULL));
	ASSERT_EQ(5, id);

	ASSERT_FALSE(AttributeSet_Contains(set, 10, &idx));
	ASSERT_EQ(ATTRIBUTE_NOTFOUND, AttributeSet_GetAttr(set, 10));

	//--------------------------------------------------------------------------
	// replace 1 attribute
	//--------------------------------------------------------------------------

	SIValue replacement = SI_LongVal(1);
	set = AttributeSet_SetAttr(&set, 0, replacement);
	ASSERT_EQ(2, AttributeSet_AttributeCount(set));
	ret = AttributeSet_GetAttr(set, 0);
	ASSERT_EQ(0, SIValue_Compare(replacement, *ret, NULL));

	// clean up
	AttributeSet_Free(set);
}

TEST_F(AttributeSetTest, RemoveAttribute) {
	AttributeSet set = AttributeSet_New(0);

	//--------------------------------------------------------------------------
	// populate AttributeSet
	//--------------------------------------------------------------------------

	SIValue attr1 = SI_ConstStringVal("attr1");
	SIValue attr2 = SI_LongVal(2);
	set = AttributeSet_SetAttr(&set, 0, attr1);
	set = AttributeSet_SetAttr(&set, 5, attr2);

	//--------------------------------------------------------------------------
	// remove real attribute
	//--------------------------------------------------------------------------

	bool removed;
	set = AttributeSet_RemoveAttr(&set, 5, &removed);
	ASSERT_TRUE(removed);

	//--------------------------------------------------------------------------
	// remove non-existent attribute
	//--------------------------------------------------------------------------

	set = AttributeSet_RemoveAttr(&set, 10, &removed);
	ASSERT_FALSE(removed);

	//--------------------------------------------------------------------------
	// remove all attributes
	//--------------------------------------------------------------------------

	uint count;
	set = AttributeSet_Clear(&set, &count);
	ASSERT_EQ(1, count);

	//--------------------------------------------------------------------------
	// re-remove attribute
	//--------------------------------------------------------------------------

	set = AttributeSet_RemoveAttr(&set, 0, &removed);
	ASSERT_FALSE(removed);

	// clean up
	AttributeSet_Free(set);
}

TEST_F(AttributeSetTest, CloneAttributeSet) {
	AttributeSet set = AttributeSet_New(0);

	//--------------------------------------------------------------------------
	// populate AttributeSet
	//--------------------------------------------------------------------------

	SIValue attr1 = SI_ConstStringVal("attr1");
	SIValue attr2 = SI_LongVal(2);
	set = AttributeSet_SetAttr(&set, 0, attr1);
	set = AttributeSet_SetAttr(&set, 5, attr2);

	//--------------------------------------------------------------------------
	// clone AttributeSet
	//--------------------------------------------------------------------------

	AttributeSet new_set = AttributeSet_Clone(set, 1);

	//--------------------------------------------------------------------------
	// validate clone contents
	//--------------------------------------------------------------------------

	ASSERT_EQ(2, AttributeSet_AttributeCount(new_set));

	uint idx;
	ASSERT_TRUE(AttributeSet_Contains(new_set, 0, &idx));
	ASSERT_EQ(0, idx);
	SIValue *ret = AttributeSet_GetAttr(new_set, 0);
	ASSERT_EQ(0, SIValue_Compare(attr1, *ret, NULL));
	Attribute_ID id;
	AttributeSet_GetAttrIdx(new_set, 0, ret, &id);
	ASSERT_EQ(0, SIValue_Compare(attr1, *ret, NULL));
	ASSERT_EQ(0, id);

	ASSERT_TRUE(AttributeSet_Contains(new_set, 5, &idx));
	ASSERT_EQ(1, idx);
	ret = AttributeSet_GetAttr(new_set, 5);
	ASSERT_EQ(0, SIValue_Compare(attr2, *ret, NULL));
	AttributeSet_GetAttrIdx(new_set, 1, ret, &id);
	ASSERT_EQ(0, SIValue_Compare(attr2, *ret, NULL));
	ASSERT_EQ(5, id);

	ASSERT_FALSE(AttributeSet_Contains(new_set, 10, &idx));
	ASSERT_EQ(ATTRIBUTE_NOTFOUND, AttributeSet_GetAttr(new_set, 10));

	// clean up
	AttributeSet_Free(set);
	AttributeSet_Free(new_set);
}

