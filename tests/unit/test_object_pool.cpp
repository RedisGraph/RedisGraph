/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../src/util/arr.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/object_pool/object_pool.h"

#ifdef __cplusplus
}
#endif

class ObjectPoolTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
	}
};


TEST_F(ObjectPoolTest, New) {
	// Create a new ObjectPool capable of holding at least 1024 integer items.
	uint item_size = sizeof(uint);
	ObjectPool *object_pool = ObjectPool_New(1024, item_size, NULL);

	ASSERT_EQ(object_pool->itemCount, 0);     // No items have been added.
	ASSERT_GE(object_pool->itemCap, 1024);
	// The size of items in the pool is greater to accommodate an internal header ID.
	uint header_size = sizeof(uint64_t);
	ASSERT_EQ(object_pool->itemSize, item_size + header_size);
	ASSERT_GE(object_pool->blockCount, 1024 / POOL_BLOCK_CAP);

	// Verify that blocks are properly chained together.
	for(uint i = 0; i < object_pool->blockCount; i++) {
		Block *block = object_pool->blocks[i];
		ASSERT_EQ(block->itemSize, object_pool->itemSize);
		ASSERT_TRUE(block->data != NULL);
		if(i > 0) {
			ASSERT_EQ(object_pool->blocks[i - 1]->next, object_pool->blocks[i]);
		}
	}

	ObjectPool_Free(object_pool);
}

TEST_F(ObjectPoolTest, AddItem) {
	ObjectPool *object_pool = ObjectPool_New(256, sizeof(uint), NULL);
	uint item_count = 128;
	uint final_item_count = item_count *= 16;
	uint *item_pointers[final_item_count];
	// Add items to pool.
	for(uint i = 0; i < item_count; i++) {
		uint *item = (uint *)ObjectPool_NewItem(object_pool);
		*item = i;
		item_pointers[i] = item;
	}

	// Add enough items to cause the ObjectPool to reallocate.
	uint prev_count = object_pool->itemCount;
	item_count = final_item_count;

	// Add new items.
	for(uint i = prev_count; i < item_count; i++) {
		uint *item = (uint *)ObjectPool_NewItem(object_pool);
		*item = i;
		item_pointers[i] = item;
	}

	// Validate that no items have been modified.
	for(uint i = 0; i < item_count; i++) {
		ASSERT_EQ(*item_pointers[i], i);
	}
	ObjectPool_Free(object_pool);
}

TEST_F(ObjectPoolTest, RemoveItem) {
	ObjectPool *object_pool = ObjectPool_New(1024, sizeof(uint), NULL);
	uint item_count = 32;

	uint *item_pointers[item_count];
	// Set items.
	for(uint i = 0; i < item_count; i++) {
		uint *item = (uint *)ObjectPool_NewItem(object_pool);
		*item = i;
		item_pointers[i] = item;
	}

	// Validate that no items have been modified.
	for(uint i = 0; i < item_count; i++) {
		ASSERT_EQ(*item_pointers[i], i);
	}

	// Delete an entry from the pool and perform validations..
	uint delete_idx = 10;
	ObjectPool_DeleteItem(object_pool, item_pointers[delete_idx]);
	ASSERT_EQ(object_pool->itemCount, item_count - 1);
	ASSERT_EQ(array_len(object_pool->deletedIdx), 1);
	// Index 10 should be added to ObjectPool deletedIdx array.
	ASSERT_EQ(object_pool->deletedIdx[0], 10);

	// Add a new item and verify that the deleted entry is reused.
	uint *new_item = (uint *)ObjectPool_NewItem(object_pool);
	ASSERT_EQ(object_pool->itemCount, item_count);
	ASSERT_EQ(array_len(object_pool->deletedIdx), 0);
	// Verify that the new entry is zeroed.
	ASSERT_EQ(*new_item, 0);

	// Cleanup.
	ObjectPool_Free(object_pool);
}

