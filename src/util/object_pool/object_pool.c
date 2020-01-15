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
    ceil((double)n / POOL_BLOCK_CAP)

// Computes block index from item index.
#define ITEM_INDEX_TO_BLOCK_INDEX(idx) \
    (idx / POOL_BLOCK_CAP)

// Computes item position within a block.
#define ITEM_POSITION_WITHIN_BLOCK(idx) \
    (idx % POOL_BLOCK_CAP)

// Get currently active block.
#define ACTIVE_BLOCK(pool) \
    pool->blocks[ITEM_INDEX_TO_BLOCK_INDEX(pool->itemCount)]

// Retrieves block in which item with index resides.
#define GET_ITEM_BLOCK(pool, idx) \
    pool->blocks[ITEM_INDEX_TO_BLOCK_INDEX(idx)]

//------------------------------------------------------------------------------
// ObjectPoolBlock
//------------------------------------------------------------------------------

static inline ObjectPoolBlock *_Block_New(uint itemSize) {
	assert(itemSize > 0);
	ObjectPoolBlock *block = rm_calloc(1, sizeof(ObjectPoolBlock) + POOL_BLOCK_CAP * itemSize);
	block->itemSize = itemSize;
	return block;
}

static inline void _Block_Free(ObjectPoolBlock *block) {
	assert(block);
	rm_free(block);
}

//------------------------------------------------------------------------------
// ObjectPool
//------------------------------------------------------------------------------

static void _ObjectPool_AddBlocks(ObjectPool *pool, uint blockCount) {
	assert(pool && blockCount > 0);

	uint prevBlockCount = pool->blockCount;
	pool->blockCount += blockCount;
	if(!pool->blocks) pool->blocks = rm_malloc(sizeof(ObjectPoolBlock *) * pool->blockCount);
	else pool->blocks = rm_realloc(pool->blocks, sizeof(ObjectPoolBlock *) * pool->blockCount);

	uint i;
	for(i = prevBlockCount; i < pool->blockCount; i++) {
		pool->blocks[i] = _Block_New(pool->itemSize);
		if(i > 0) pool->blocks[i - 1]->next = pool->blocks[i];
	}
	pool->blocks[i - 1]->next = NULL;

	pool->itemCap = pool->blockCount * POOL_BLOCK_CAP;
}

static inline void _ObjectPool_ClearItem(const ObjectPool *pool, unsigned char *item) {
	memset(item, 0, pool->itemSize);
}

ObjectPool *ObjectPool_New(uint itemCap, uint itemSize, fpDestructor fp) {
	ObjectPool *pool = rm_malloc(sizeof(ObjectPool));
	pool->itemCount = 0;
	pool->itemSize = itemSize;
	pool->blockCount = 0;
	pool->blocks = NULL;
	pool->deletedIdx = array_new(uint, 128);
	pool->destructor = fp;
	_ObjectPool_AddBlocks(pool, ITEM_COUNT_TO_BLOCK_COUNT(itemCap));
	return pool;
}

/*
// Make sure datablock can accommodate at least k items.
void ObjectPool_Accommodate(ObjectPool *pool, uint k) {
	// Compute number of free slots.
	int64_t freeSlotsCount = pool->itemCap - pool->itemCount;
	int64_t additionalItems = k - freeSlotsCount;

	if(additionalItems > 0) {
		int64_t additionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(additionalItems);
		_ObjectPool_AddBlocks(pool, additionalBlocks);
	}
}
*/

/*
void *ObjectPool_GetItem(const ObjectPool *pool, uint idx) {
	assert(pool);

	ObjectPoolBlock *block = GET_ITEM_BLOCK(pool, idx);
	idx = ITEM_POSITION_WITHIN_BLOCK(idx);

	unsigned char *item = block->data + (idx * block->itemSize);

	return item;
}
*/

void *ObjectPool_AllocateItem(ObjectPool *pool, uint *idx) {
	// Make sure we've got room for items.
	if(pool->itemCount >= pool->itemCap) { // TODO can check for deleted indices first.
		// Allocate twice as much items then we currently hold.
		uint newCap = pool->itemCount * 2;
		uint requiredAdditionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(newCap) - pool->blockCount;
		_ObjectPool_AddBlocks(pool, requiredAdditionalBlocks);
	}

	// Get index into which to store item,
	// prefer reusing free indicies.
	uint pos = (array_len(pool->deletedIdx)) ? array_pop(pool->deletedIdx) : pool->itemCount;
	pool->itemCount++;

	if(idx) *idx = pos;

	ObjectPoolBlock *block = GET_ITEM_BLOCK(pool, pos);
	pos = ITEM_POSITION_WITHIN_BLOCK(pos);

	unsigned char *item = block->data + (pos * block->itemSize);

	return item;
}

void ObjectPool_DeleteItem(ObjectPool *pool, uint idx) {
	assert(pool);

	// Get block.
	uint blockIdx = ITEM_INDEX_TO_BLOCK_INDEX(idx);
	ObjectPoolBlock *block = pool->blocks[blockIdx];

	uint blockPos = ITEM_POSITION_WITHIN_BLOCK(idx);
	uint offset = blockPos * block->itemSize;

	// Call item destructor.
	unsigned char *item = block->data + offset;
	if(pool->destructor) pool->destructor(item);
	_ObjectPool_ClearItem(pool, item); // TODO consider combining with destructor

	pool->deletedIdx = array_append(pool->deletedIdx, idx);
	pool->itemCount--;
}

void ObjectPool_Free(ObjectPool *pool) {
	for(int i = 0; i < pool->blockCount; i++) {
		_Block_Free(pool->blocks[i]);
	}

	rm_free(pool->blocks);
	array_free(pool->deletedIdx);
	rm_free(pool);
}

