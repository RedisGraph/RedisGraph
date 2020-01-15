/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef void (*fpDestructor)(void *);

// Number of items in a block. Should always be a power of 2.
#define POOL_BLOCK_CAP 256

/* Data block is a type agnostic continuous block of memory
 * used to hold items of the same type, each block has a next
 * pointer to another block or NULL if this is the last block. */

typedef struct ObjectPoolBlock {
	size_t itemSize;        // Size of a single Item in bytes.
	struct ObjectPoolBlock *next;     // Pointer to next block.
	unsigned char data[];   // Item array. MUST BE LAST MEMBER OF THE STRUCT!
} ObjectPoolBlock;

/* Data block is a type agnostic continues block of memory
 * used to hold items of the same type, each block has a next
 * pointer to another block or NULL if this is the last block. */
typedef struct {
	uint itemCount;             // Number of items stored in datablock.
	uint itemCap;               // Number of items datablock can hold.
	uint blockCount;            // Number of blocks in datablock.
	uint itemSize;              // Size of a single Item in bytes.
	ObjectPoolBlock **blocks;   // Array of blocks.
	uint *deletedIdx;           // Array of free indicies.
	fpDestructor destructor;    // Function pointer to a clean-up function of an item.
} ObjectPool;

// Create a new ObjectPool
// itemCap - number of items datablock can hold before resizing.
// itemSize - item size in bytes.
ObjectPool *ObjectPool_New(uint itemCap, uint itemSize, fpDestructor fp);

// Make sure datablock can accommodate at least k items.
// void ObjectPool_Accommodate(ObjectPool *pool, uint k);

// Get item at position idx
// void *ObjectPool_GetItem(const ObjectPool *pool, uint idx);

// Allocate a new item within given pool,
// if idx is not NULL, idx will contain item position
// return a pointer to the newly allocated item.
void *ObjectPool_AllocateItem(ObjectPool *pool, uint *idx);

// Removes item at position idx.
void ObjectPool_DeleteItem(ObjectPool *pool, uint idx);

// Free block.
void ObjectPool_Free(ObjectPool *block);

