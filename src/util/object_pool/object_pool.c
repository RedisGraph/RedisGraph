/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "../arr.h"
#include "../block.h"
#include "../rmalloc.h"
#include "object_pool.h"
#include <math.h>
#include <string.h>
#include <assert.h>

// Computes the number of blocks required to accommodate n items.
#define ITEM_COUNT_TO_BLOCK_COUNT(n) \
	ceil((double)n / POOL_BLOCK_CAP)

// Computes block index from item index.
#define ITEM_INDEX_TO_BLOCK_INDEX(idx) \
	(idx / POOL_BLOCK_CAP)

// Computes item position within a block.
#define ITEM_POSITION_WITHIN_BLOCK(idx) \
	(idx % POOL_BLOCK_CAP)

// Retrieves block in which item with index resides.
#define GET_ITEM_BLOCK(pool, idx) \
	pool->blocks[ITEM_INDEX_TO_BLOCK_INDEX(idx)]

static void _ObjectPool_AddBlocks(ObjectPool *pool, uint blockCount) {
	assert(pool && blockCount > 0);

	uint prevBlockCount = pool->blockCount;
	pool->blockCount += blockCount;
	if(!pool->blocks) pool->blocks = rm_malloc(sizeof(Block *) * pool->blockCount);
	else pool->blocks = rm_realloc(pool->blocks, sizeof(Block *) * pool->blockCount);

	uint i;
	for(i = prevBlockCount; i < pool->blockCount; i++) {
		pool->blocks[i] = Block_New(pool->itemSize, POOL_BLOCK_CAP);
		if(i > 0) pool->blocks[i - 1]->next = pool->blocks[i];
	}
	pool->blocks[i - 1]->next = NULL;

	pool->itemCap = pool->blockCount * POOL_BLOCK_CAP;
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

void *ObjectPool_NewItem(ObjectPool *pool, uint *idx) {
	// Make sure we have room for a new item.
	if(pool->itemCount >= pool->itemCap) {
		// Double the capacity of the pool.
		_ObjectPool_AddBlocks(pool, pool->itemCap);
	}

	// Get the index of the allocation, reusing a deleted index if possible.
	uint pos = (array_len(pool->deletedIdx)) ? array_pop(pool->deletedIdx) : pool->itemCount;
	pool->itemCount++;

	*idx = pos;

	Block *block = GET_ITEM_BLOCK(pool, pos);
	pos = ITEM_POSITION_WITHIN_BLOCK(pos);

	unsigned char *item = block->data + (pos * block->itemSize);
	// Zero-set the item being returned.
	memset(item, 0, block->itemSize);

	return item;
}

void ObjectPool_DeleteItem(ObjectPool *pool, uint idx) {
	assert(pool);

	// Get block.
	uint blockIdx = ITEM_INDEX_TO_BLOCK_INDEX(idx);
	Block *block = pool->blocks[blockIdx];

	uint blockPos = ITEM_POSITION_WITHIN_BLOCK(idx);
	uint offset = blockPos * block->itemSize;

	// Call item destructor.
	unsigned char *item = block->data + offset;
	if(pool->destructor) pool->destructor(item);

	pool->deletedIdx = array_append(pool->deletedIdx, idx);
	pool->itemCount--;
}

void ObjectPool_Free(ObjectPool *pool) {
	for(uint i = 0; i < pool->blockCount; i++) Block_Free(pool->blocks[i]);

	rm_free(pool->blocks);
	array_free(pool->deletedIdx);
	rm_free(pool);
}

