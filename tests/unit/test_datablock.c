/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <string.h>
#include "./acutest.h"
#include "../../src/util/arr.h"
#include "../../src/util/datablock/datablock.h"
#include "../../src/util/datablock/oo_datablock.h"
#include "../../src/util/rmalloc.h"

static void setup(void) {
	// Use the malloc family for allocations
	Alloc_Reset();
}


void test_New(void) {
	setup();

	// Create a new data block, which can hold at least 1024 items
	// each item is an integer.
	size_t itemSize = sizeof(int);
	DataBlock *dataBlock = DataBlock_New(1024, itemSize, NULL);

	TEST_CHECK(dataBlock->itemCount == 0);     // No items were added.
	TEST_CHECK(dataBlock->itemCap >= 1024);
	TEST_CHECK(dataBlock->itemSize == itemSize + ITEM_HEADER_SIZE);
	TEST_CHECK((double)dataBlock->blockCount >= (1024 / DATABLOCK_BLOCK_CAP));

	for(int i = 0; i < dataBlock->blockCount; i++) {
		Block *block = dataBlock->blocks[i];
		TEST_CHECK(block->itemSize == dataBlock->itemSize);
		TEST_CHECK(block->data != NULL);
		if(i > 0) {
			TEST_CHECK(dataBlock->blocks[i - 1]->next == dataBlock->blocks[i]);
		}
	}

	DataBlock_Free(dataBlock);
}

void test_AddItem(void) {
	setup();

	DataBlock *dataBlock = DataBlock_New(1024, sizeof(int), NULL);
	size_t itemCount = 512;
	DataBlock_Accommodate(dataBlock, itemCount);
	TEST_CHECK(dataBlock->itemCap >= itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *value = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*value = i;
	}

	// Read items.
	for(int i = 0 ; i < itemCount; i++) {
		int *value = (int *)DataBlock_GetItem(dataBlock, i);
		TEST_CHECK(*value == i);
	}

	// Add enough item to cause datablock to re-allocate.
	size_t prevItemCount = dataBlock->itemCount;
	itemCount *= 64;
	DataBlock_Accommodate(dataBlock, itemCount);
	TEST_CHECK(dataBlock->itemCap >= prevItemCount + itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *value = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*value = prevItemCount + i;
	}

	// Read items.
	for(int i = 0 ; i < dataBlock->itemCount; i++) {
		int *value = (int *)DataBlock_GetItem(dataBlock, i);
		TEST_CHECK(*value == i);
	}

	DataBlock_Free(dataBlock);
}

void test_Scan(void) {
	setup();

	DataBlock *dataBlock = DataBlock_New(1024, sizeof(int), NULL);
	size_t itemCount = 2048;
	DataBlock_Accommodate(dataBlock, itemCount);

	// Set items.
	for(int i = 0 ; i < itemCount; i++) {
		int *item = (int *)DataBlock_AllocateItem(dataBlock, NULL);
		*item = i;
	}

	// Scan through items.
	int count = 0;      // items iterated so far
	int *item = NULL;   // current iterated item
	uint64_t idx = 0;   // iterated item index

	DataBlockIterator *it = DataBlock_Scan(dataBlock);
	while((item = (int *)DataBlockIterator_Next(it, &idx))) {
		TEST_CHECK(count == idx);
		TEST_CHECK(*item == count);
		count++;
	}
	TEST_CHECK(count == itemCount);

	item = (int *)DataBlockIterator_Next(it, NULL);
	TEST_CHECK(item == NULL);

	DataBlockIterator_Reset(it);

	// Re-scan through items.
	count = 0;
	item = NULL;
	while((item = (int *)DataBlockIterator_Next(it, &idx))) {
		TEST_CHECK(count == idx);
		TEST_CHECK(*item == count);
		count++;
	}
	TEST_CHECK(count == itemCount);

	item = (int *)DataBlockIterator_Next(it, NULL);
	TEST_CHECK(item == NULL);

	DataBlock_Free(dataBlock);
	DataBlockIterator_Free(it);
}

void test_RemoveItem(void) {
	setup();

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
	TEST_CHECK(!(item == NULL));
	TEST_CHECK(*item == 0);

	// Remove item at position 0 and perform validations
	// Index 0 should be added to datablock deletedIdx array.
	DataBlock_DeleteItem(dataBlock, 0);
	TEST_CHECK(dataBlock->itemCount == itemCount - 1);
	TEST_CHECK(array_len(dataBlock->deletedIdx) == 1);
	DataBlockItemHeader *header = (DataBlockItemHeader *)dataBlock->blocks[0]->data;
	TEST_CHECK(IS_ITEM_DELETED(header));

	// Try to get item from deleted cell.
	item = (int *)DataBlock_GetItem(dataBlock, 0);
	TEST_CHECK(item == NULL);

	// Iterate over datablock, deleted item should be skipped.
	DataBlockIterator *it = DataBlock_Scan(dataBlock);
	uint counter = 0;
	while(DataBlockIterator_Next(it, NULL)) counter++;
	TEST_CHECK(counter == itemCount - 1);
	DataBlockIterator_Free(it);

	// There's no harm in deleting a deleted item.
	DataBlock_DeleteItem(dataBlock, 0);

	// Add a new item, expecting deleted cell to be reused.
	int *newItem = (int *)DataBlock_AllocateItem(dataBlock, NULL);
	TEST_CHECK(dataBlock->itemCount == itemCount);
	TEST_CHECK(array_len(dataBlock->deletedIdx) == 0);
	TEST_CHECK((void *)newItem == (void *)((dataBlock->blocks[0]->data) + ITEM_HEADER_SIZE));

	it = DataBlock_Scan(dataBlock);
	counter = 0;
	while(DataBlockIterator_Next(it, NULL)) counter++;
	TEST_CHECK(counter == itemCount);
	DataBlockIterator_Free(it);

	// Cleanup.
	DataBlock_Free(dataBlock);
}

void test_OutOfOrderBuilding(void) {
	setup();

	// This test checks for a fragmented, data block out of order re-construction.
	DataBlock *dataBlock = DataBlock_New(1, sizeof(int), NULL);
	int insert_arr1[4] = {8, 2, 3, 6};
	int delete_arr[2] = {4, 7};
	int insert_arr2[4] = {9, 1, 5, 0};
	int expected[8] = {0, 1, 2, 3, 5, 6, 8, 9};

	// Insert the first array.
	for(int i = 0; i < 4; i++) {
		int *item = (int *)DataBlock_AllocateItemOutOfOrder(dataBlock, insert_arr1[i]);
		*item = insert_arr1[i];
	}
	TEST_CHECK(4 == dataBlock->itemCount);
	TEST_CHECK(0 == array_len(dataBlock->deletedIdx));

	// Mark deleted values.
	for(int i = 0; i < 2; i++) {
		DataBlock_MarkAsDeletedOutOfOrder(dataBlock, delete_arr[i]);
	}

	TEST_CHECK(4 == dataBlock->itemCount);
	TEST_CHECK(2 == array_len(dataBlock->deletedIdx));

	// Add another array
	for(int i = 0; i < 4; i++) {
		int *item = (int *)DataBlock_AllocateItemOutOfOrder(dataBlock, insert_arr2[i]);
		*item = insert_arr2[i];
	}

	TEST_CHECK(8 == dataBlock->itemCount);
	TEST_CHECK(2 == array_len(dataBlock->deletedIdx));

	// Validate
	DataBlockIterator *it = DataBlock_Scan(dataBlock);
	for(int i = 0; i < 8; i++) {
		int *item = (int *)DataBlockIterator_Next(it, NULL);
		TEST_CHECK(*item == expected[i]);
	}
	TEST_CHECK(!DataBlockIterator_Next(it, NULL));

	TEST_CHECK(dataBlock->deletedIdx[0] == 4 || dataBlock->deletedIdx[0] == 7);
	TEST_CHECK(dataBlock->deletedIdx[1] == 4 || dataBlock->deletedIdx[1] == 7);

	DataBlock_Free(dataBlock);
}

TEST_LIST = {
	{ "test_AddItem", test_AddItem },
	{ "test_New", test_New },
	{ "test_OutOfOrderBuilding", test_OutOfOrderBuilding },
	{ "test_RemoveItem", test_RemoveItem },
	{ "test_Scan", test_Scan },
	{ NULL, NULL }     // zeroed record marking the end of the list
};

