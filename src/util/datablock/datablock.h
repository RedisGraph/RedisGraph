/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdlib.h>
#include <pthread.h>
#include "../block.h"
#include "./datablock_iterator.h"

typedef void (*fpDestructor)(void *);

// Number of items in a block. Should always be a power of 2.
#define DATABLOCK_BLOCK_CAP 16384

// Returns the item header size.
#define ITEM_HEADER_SIZE 1

// DataBlock item is stored as ||header|data||. This macro retrive the data pointer out of the header pointer.
#define ITEM_DATA(header) ((void *)((header) + ITEM_HEADER_SIZE))

// DataBlock item is stored as ||header|data||. This macro retrive the header pointer out of the data pointer.
#define GET_ITEM_HEADER(item) ((item) - ITEM_HEADER_SIZE)

// Sets the deleted bit in the header to 1.
#define MARK_HEADER_AS_DELETED(header) ((header)->deleted |= 1)

// Sets the deleted bit in the header to 0.
#define MARK_HEADER_AS_NOT_DELETED(header) ((header)->deleted &= 0)

// Checks if the deleted bit in the header is 1 or not.
#define IS_ITEM_DELETED(header) ((header)->deleted & 1)


/* The DataBlock is a container structure for holding arbitrary items of a uniform type
 * in order to reduce the number of alloc/free calls and improve locality of reference.
 * Item deletions are thread-safe, and a DataBlockIterator can be used to traverse a
 * range within the block. */
typedef struct {
	uint64_t itemCount;         // Number of items stored in datablock.
	uint64_t itemCap;           // Number of items datablock can hold.
	uint blockCount;            // Number of blocks in datablock.
	uint itemSize;              // Size of a single item in bytes.
	Block **blocks;             // Array of blocks.
	uint64_t *deletedIdx;       // Array of free indicies.
	pthread_mutex_t mutex;      // Mutex guarding from concurent updates.
	fpDestructor destructor;    // Function pointer to a clean-up function of an item.
} DataBlock;

// This struct is for data block item header data.
// TODO: Consider using pragma pack/pop for tight memory/word alignment.
typedef struct {
	unsigned char deleted: 1;  // A bit indicate if the current item is deleted or not.
} DataBlockItemHeader;

// Create a new DataBlock
// itemCap - number of items datablock can hold before resizing.
// itemSize - item size in bytes.
// fp - destructor routine for freeing items.
DataBlock *DataBlock_New(uint64_t itemCap, uint itemSize, fpDestructor fp);

// Make sure datablock can accommodate at least k items.
void DataBlock_Accommodate(DataBlock *dataBlock, int64_t k);

// Returns an iterator which scans entire datablock.
DataBlockIterator *DataBlock_Scan(const DataBlock *dataBlock);

// Get item at position idx
void *DataBlock_GetItem(const DataBlock *dataBlock, uint64_t idx);

// Allocate a new item within given dataBlock,
// if idx is not NULL, idx will contain item position
// return a pointer to the newly allocated item.
void *DataBlock_AllocateItem(DataBlock *dataBlock, uint64_t *idx);

// Removes item at position idx.
void DataBlock_DeleteItem(DataBlock *dataBlock, uint64_t idx);

// Returns the number of deleted items.
uint DataBlock_DeletedItemsCount(const DataBlock *dataBlock);

// Free block.
void DataBlock_Free(DataBlock *block);
