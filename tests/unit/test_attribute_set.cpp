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

TEST_F(AttributeSetTest, AttributeSetNew) {

}

