/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "../arr.h"
#include "object_pool.h"
#include "../rmalloc.h"

// Computes the number of blocks required to accommodate n items.
#define ITEM_COUNT_TO_BLOCK_COUNT(n) \
    ceil((double)n / BLOCK_CAP)

// Computes block index from item index.
#define ITEM_INDEX_TO_BLOCK_INDEX(idx) \
    (idx / BLOCK_CAP)

// Computes item position within a block.
#define ITEM_POSITION_WITHIN_BLOCK(idx) \
    (idx % BLOCK_CAP)

// Get currently active block.
#define ACTIVE_BLOCK(dataBlock) \
    dataBlock->blocks[ITEM_INDEX_TO_BLOCK_INDEX(dataBlock->itemCount)]

// Retrieves block in which item with index resides.
#define GET_ITEM_BLOCK(dataBlock, idx) \
    dataBlock->blocks[ITEM_INDEX_TO_BLOCK_INDEX(idx)]

//------------------------------------------------------------------------------
// Block
//------------------------------------------------------------------------------

static inline Block *_Block_New(uint itemSize) {
	assert(itemSize > 0);
	Block *block = rm_calloc(1, sizeof(Block) + BLOCK_CAP * itemSize);
	block->itemSize = itemSize;
	return block;
}

static inline void _Block_Free(Block *block) {
	assert(block);
	rm_free(block);
}

//------------------------------------------------------------------------------
// ObjectPool
//------------------------------------------------------------------------------

void _ObjectPool_AddBlocks(ObjectPool *dataBlock, uint blockCount) {
	assert(dataBlock && blockCount > 0);

	uint prevBlockCount = dataBlock->blockCount;
	dataBlock->blockCount += blockCount;
	if(!dataBlock->blocks)
		dataBlock->blocks = rm_malloc(sizeof(Block *) * dataBlock->blockCount);
	else
		dataBlock->blocks = rm_realloc(dataBlock->blocks, sizeof(Block *) * dataBlock->blockCount);

	uint i;
	for(i = prevBlockCount; i < dataBlock->blockCount; i++) {
		dataBlock->blocks[i] = _Block_New(dataBlock->itemSize);
		if(i > 0) dataBlock->blocks[i - 1]->next = dataBlock->blocks[i];
	}
	dataBlock->blocks[i - 1]->next = NULL;

	dataBlock->itemCap = dataBlock->blockCount * BLOCK_CAP;
}

static inline void _ObjectPool_MarkItemAsDeleted(const ObjectPool *dataBlock, unsigned char *item) {
	memset(item, 0, dataBlock->itemSize);
}

static inline void _ObjectPool_MarkItemAsUndelete(const ObjectPool *dataBlock,
												  unsigned char *item) {
	// item[0] = !dataBlock->deleted_marker;
}

static inline int _ObjectPool_IsItemDeleted(const ObjectPool *dataBlock, unsigned char *item) {
	for(uint i = 0; i < dataBlock->itemSize; i++) {
		if(item[i] != dataBlock->deleted_marker) return 0;
	}
	return 1;
}

// Checks to see if idx is within global array bounds
// array bounds are between 0 and itemCount + #deleted indices
// e.g. [3, 7, 2, D, 1, D, 5] where itemCount = 5 and #deleted indices is 2
// and so it is valid to query the array with idx 6.
static inline int _ObjectPool_IndexOutOfBounds(const ObjectPool *dataBlock, uint idx) {
	return (idx >= (dataBlock->itemCount + array_len(dataBlock->deletedIdx)));
}

ObjectPool *ObjectPool_New(uint itemCap, uint itemSize, fpDestructor fp) {
	ObjectPool *dataBlock = rm_malloc(sizeof(ObjectPool));
	dataBlock->itemCount = 0;
	dataBlock->itemSize = itemSize;
	dataBlock->blockCount = 0;
	dataBlock->blocks = NULL;
	dataBlock->deletedIdx = array_new(uint, 128);
	dataBlock->destructor = fp;
	dataBlock->deleted_marker = deleted_marker;
	_ObjectPool_AddBlocks(dataBlock, ITEM_COUNT_TO_BLOCK_COUNT(itemCap));
	return dataBlock;
}

// Make sure datablock can accommodate at least k items.
void ObjectPool_Accommodate(ObjectPool *dataBlock, int64_t k) {
	// Compute number of free slots.
	int64_t freeSlotsCount = dataBlock->itemCap - dataBlock->itemCount;
	int64_t additionalItems = k - freeSlotsCount;

	if(additionalItems > 0) {
		int64_t additionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(additionalItems);
		_ObjectPool_AddBlocks(dataBlock, additionalBlocks);
	}
}

void *ObjectPool_GetItem(const ObjectPool *dataBlock, uint idx) {
	assert(dataBlock);

	if(_ObjectPool_IndexOutOfBounds(dataBlock, idx)) return NULL;

	Block *block = GET_ITEM_BLOCK(dataBlock, idx);
	idx = ITEM_POSITION_WITHIN_BLOCK(idx);

	unsigned char *item = block->data + (idx * block->itemSize);

	// Incase item is marked as deleted, return NULL.
	if(_ObjectPool_IsItemDeleted(dataBlock, item)) return NULL;

	return item;
}

void *ObjectPool_AllocateItem(ObjectPool *dataBlock, uint *idx) {
	// Make sure we've got room for items.
	if(dataBlock->itemCount >= dataBlock->itemCap) { // TODO can check for deleted indices first.
		// Allocate twice as much items then we currently hold.
		uint newCap = dataBlock->itemCount * 2;
		uint requiredAdditionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(newCap) - dataBlock->blockCount;
		_ObjectPool_AddBlocks(dataBlock, requiredAdditionalBlocks);
	}

	// Get index into which to store item,
	// prefer reusing free indicies.
	uint pos = dataBlock->itemCount;
	if(array_len(dataBlock->deletedIdx) > 0) {
		pos = array_pop(dataBlock->deletedIdx);
	}
	dataBlock->itemCount++;

	if(idx) *idx = pos;

	Block *block = GET_ITEM_BLOCK(dataBlock, pos);
	pos = ITEM_POSITION_WITHIN_BLOCK(pos);

	unsigned char *item = block->data + (pos * block->itemSize);
	_ObjectPool_MarkItemAsUndelete(dataBlock, item);

	return (void *)item;
}

void ObjectPool_DeleteItem(ObjectPool *dataBlock, uint idx) {
	assert(dataBlock);
	if(_ObjectPool_IndexOutOfBounds(dataBlock, idx)) return;

	// Get block.
	uint blockIdx = ITEM_INDEX_TO_BLOCK_INDEX(idx);
	Block *block = dataBlock->blocks[blockIdx];

	uint blockPos = ITEM_POSITION_WITHIN_BLOCK(idx);
	uint offset = blockPos * block->itemSize;

	// Return if item already deleted.
	unsigned char *item = block->data + offset;
	if(_ObjectPool_IsItemDeleted(dataBlock, item)) return;

	// Call item destructor.
	if(dataBlock->destructor) dataBlock->destructor(item);

	_ObjectPool_MarkItemAsDeleted(dataBlock, item);

	/* ObjectPool_DeleteItem should be thread-safe as it's being called
	 * from GraphBLAS concurent operations, e.g. GxB_SelectOp.
	 * As such updateing the datablock deleted indices array must be guarded
	 * if there's enough space to accommodate the deleted idx the operation should
	 * return quickly otherwise, memory reallocation will occur, which we want to perform
	 * in a thread safe matter. */
	dataBlock->deletedIdx = array_append(dataBlock->deletedIdx, idx);
	dataBlock->itemCount--;
}

void ObjectPool_Free(ObjectPool *dataBlock) {
	for(int i = 0; i < dataBlock->blockCount; i++)
		_Block_Free(dataBlock->blocks[i]);

	rm_free(dataBlock->blocks);
	array_free(dataBlock->deletedIdx);
	rm_free(dataBlock);
}

