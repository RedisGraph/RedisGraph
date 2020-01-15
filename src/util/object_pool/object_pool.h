/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdlib.h>
#include <stdint.h>

// Number of items in a block. Should always be a power of 2.
#define POOL_BLOCK_CAP 256

/* The ObjectPool, like the DataBlock, is a container structure for holding arbitrary items of a uniform type
 * in order to reduce the number of alloc/free calls and improve locality of reference.
 * In contrast to the DataBlock, deletions are not thread-safe. */
typedef struct {
	uint itemCount;             // Number of items stored in ObjectPool.
	uint itemCap;               // Number of items ObjectPool can hold.
	uint blockCount;            // Number of blocks in ObjectPool.
	uint itemSize;              // Size of a single item in bytes.
	Block **blocks;             // Array of blocks.
	uint *deletedIdx;           // Array of free indices.
	fpDestructor destructor;    // Function pointer to a clean-up function of an item.
} ObjectPool;

// Create a new ObjectPool
// itemCap - number of items ObjectPool can hold before resizing.
// itemSize - item size in bytes.
// fp - destructor routine for freeing items.
ObjectPool *ObjectPool_New(uint itemCap, uint itemSize, fpDestructor fp);

// Allocate a new item within the given pool.
// idx will be set to the item's position within the pool's blocks,
// and a pointer to the item will be returned.
void *ObjectPool_NewItem(ObjectPool *pool, uint *idx);

// Removes item at position idx.
void ObjectPool_DeleteItem(ObjectPool *pool, uint idx);

// Free block.
void ObjectPool_Free(ObjectPool *block);

