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
#include "./block.h"

typedef void (*fpDestructor)(void *);

/* Data block is a type agnostic continues block of memory
 * used to hold items of the same type, each block has a next
 * pointer to another block or NULL if this is the last block. */
typedef struct ObjectPool {
	uint itemCount;           // Number of items stored in datablock.
	uint itemCap;             // Number of items datablock can hold.
	uint blockCount;          // Number of blocks in datablock.
	uint itemSize;            // Size of a single Item in bytes.
	Block **blocks;             // Array of blocks.
	uint *deletedIdx;       // Array of free indicies.
	fpDestructor destructor;    // Function pointer to a clean-up function of an item.
} ObjectPool;

// Create a new ObjectPool
// itemCap - number of items datablock can hold before resizing.
// itemSize - item size in bytes.
ObjectPool *ObjectPool_New(uint itemCap, uint itemSize, fpDestructor fp);

// Make sure datablock can accommodate at least k items.
void ObjectPool_Accommodate(ObjectPool *dataBlock, uint k);

// Get item at position idx
void *ObjectPool_GetItem(const ObjectPool *dataBlock, uint idx);

// Allocate a new item within given dataBlock,
// if idx is not NULL, idx will contain item position
// return a pointer to the newly allocated item.
void *ObjectPool_AllocateItem(ObjectPool *dataBlock, uint *idx);

// Removes item at position idx.
void ObjectPool_DeleteItem(ObjectPool *dataBlock, uint idx);

// Free block.
void ObjectPool_Free(ObjectPool *block);

