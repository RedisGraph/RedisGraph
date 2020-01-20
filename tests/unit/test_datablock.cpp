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
#include <string.h>
#include "../../src/util/arr.h"
#include "../../src/util/datablock/datablock.h"
#include "../../src/util/datablock/datablock_defs.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

class DataBlockTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
	}
};


TEST_F(DataBlockTest, New) {
	// Create a new data block, which can hold at least 1024 items
	// each item is an integer.
	size_t itemSize = sizeof(int);
	DataBlock *dataBlock = DataBlock_New(1024, itemSize, NULL);

	ASSERT_EQ(dataBlock->itemCount, 0);     // No items were added.
	ASSERT_GE(dataBlock->itemCap, 1024);
	ASSERT_EQ(dataBlock->itemSize, itemSize + ITEM_HEADER_SIZE);
	ASSERT_GE(dataBlock->blockCount, 1024 / DATABLOCK_BLOCK_CAP);

	for(int i = 0; i < dataBlock->blockCount; i++) {
		Block *block = dataBlock->blocks[i];
		ASSERT_EQ(block->itemSize, dataBlock->itemSize);
		ASSERT_TRUE(block->data != NULL);
		if(i > 0) {
			ASSERT_TRUE(dataBlock->blocks[i - 1]->next == dataBlock->blocks[i]);
		}
	}

	DataBlock_Free(dataBlock);
}

TEST_F(DataBlockTest, AddItem) {
	DataBlock *dataBlock = DataBlock_New(1024, sizeof(int), NULL);
	size_t itemCount = 512;
	DataBlock_Accommodate(dataBlock, itemCount);
	ASSERT_GE(dataBlock->itemCap, itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *value = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*value = i;
	}

	// Read items.
	for(int i = 0 ; i < itemCount; i++) {
		int *value = (int *)DataBlock_GetItem(dataBlock, i);
		ASSERT_EQ(*value, i);
	}

	// Add enough item to cause datablock to re-allocate.
	size_t prevItemCount = dataBlock->itemCount;
	itemCount *= 64;
	DataBlock_Accommodate(dataBlock, itemCount);
	ASSERT_GE(dataBlock->itemCap, prevItemCount + itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *value = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*value = prevItemCount + i;
	}

	// Read items.
	for(int i = 0 ; i < dataBlock->itemCount; i++) {
		int *value = (int *)DataBlock_GetItem(dataBlock, i);
		ASSERT_EQ(*value, i);
	}

	DataBlock_Free(dataBlock);
}

TEST_F(DataBlockTest, Scan) {
	DataBlock *dataBlock = DataBlock_New(1024, sizeof(int), NULL);
	size_t itemCount = 2048;
	DataBlock_Accommodate(dataBlock, itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *item = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*item = i;
	}

	// Scan through items.
	int count = 0;
	int *item = NULL;
	DataBlockIterator *it = DataBlock_Scan(dataBlock);
	while((item = (int *)DataBlockIterator_Next(it))) {
		ASSERT_EQ(*item, count);
		count++;
	}
	ASSERT_EQ(count, itemCount);

	item = (int *)DataBlockIterator_Next(it);
	ASSERT_TRUE(item == NULL);

	DataBlockIterator_Reset(it);

	// Re-scan through items.
	count = 0;
	item = NULL;
	while((item = (int *)DataBlockIterator_Next(it))) {
		ASSERT_EQ(*item, count);
		count++;
	}
	ASSERT_EQ(count, itemCount);

	item = (int *)DataBlockIterator_Next(it);
	ASSERT_TRUE(item == NULL);

	DataBlock_Free(dataBlock);
	DataBlockIterator_Free(it);
}

TEST_F(DataBlockTest, RemoveItem) {
	DataBlock *dataBlock = DataBlock_New(1024, sizeof(int), NULL);
	uint itemCount = 32;
	DataBlock_Accommodate(dataBlock, itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *item = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*item = i;
	}

	// Validate item at position 0.
	int *item = (int *)DataBlock_GetItem(dataBlock, 0);
	ASSERT_FALSE(item == NULL);
	ASSERT_EQ(*item, 0);

	// Remove item at position 0 and perform validations
	// A DELETED_MARKER should mark cell as deleted
	// Index 0 should be added to datablock deletedIdx array.
	DataBlock_DeleteItem(dataBlock, 0);
	ASSERT_EQ(dataBlock->itemCount, itemCount - 1);
	ASSERT_EQ(array_len(dataBlock->deletedIdx), 1);
	ASSERT_TRUE((char)dataBlock->blocks[0]->data[0] && (char)DELETED_MARKER);

	// Try to get item from deleted cell.
	item = (int *)DataBlock_GetItem(dataBlock, 0);
	ASSERT_TRUE(item == NULL);

	// Iterate over datablock, deleted item should be skipped.
	DataBlockIterator *it = DataBlock_Scan(dataBlock);
	uint counter = 0;
	while(DataBlockIterator_Next(it)) counter++;
	ASSERT_EQ(counter, itemCount - 1);
	DataBlockIterator_Free(it);

	// There's no harm in deleting a deleted item.
	DataBlock_DeleteItem(dataBlock, 0);

	// Add a new item, expecting deleted cell to be reused.
	int *newItem = (int *)DataBlock_AllocateItem(dataBlock, NULL);
	ASSERT_EQ(dataBlock->itemCount, itemCount);
	ASSERT_EQ(array_len(dataBlock->deletedIdx), 0);
	ASSERT_TRUE((void *)newItem == (void *)(&dataBlock->blocks[0]->data) + ITEM_HEADER_SIZE);

	it = DataBlock_Scan(dataBlock);
	counter = 0;
	while(DataBlockIterator_Next(it)) counter++;
	ASSERT_EQ(counter, itemCount);
	DataBlockIterator_Free(it);

	// Cleanup.
	DataBlock_Free(dataBlock);
}

