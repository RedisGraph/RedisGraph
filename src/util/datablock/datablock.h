/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _DATA_BLOCK_H_
#define _DATA_BLOCK_H_

#include <stdlib.h>
#include "./block.h"
#include "./datablock_iterator.h"

#define DELETED_MARKER 0xFF

/* Data block is a type agnostic continues block of memory 
 * used to hold items of the same type, each block has a next 
 * pointer to another block or NULL if this is the last block. */

typedef struct DataBlock {
    size_t itemCount;       // Number of items stored in datablock.
    size_t itemCap;         // Number of items datablock can hold.
    size_t blockCount;      // Number of blocks in datablock.
    size_t itemSize;        // Size of a single Item in bytes.
    Block **blocks;         // Array of blocks.
    uint64_t *deletedIdx;   // Array of free indicies.
} DataBlock;

// Create a new DataBlock
// itemCap - number of items datablock can hold before resizing.
// itemSize - item size in bytes.
DataBlock *DataBlock_New(size_t itemCap, size_t itemSize);

// Make sure datablock can accommodate at least k items.
void DataBlock_Accommodate(DataBlock *dataBlock, int64_t k);

// Returns an iterator which scans entire datablock.
DataBlockIterator *DataBlock_Scan(const DataBlock *dataBlock);

// Get item at position idx
void *DataBlock_GetItem(const DataBlock *dataBlock, size_t idx);

// Allocate a new item within given dataBlock,
// if idx is not NULL, idx will contain item position
// return a pointer to the newly allocated item.
void* DataBlock_AllocateItem(DataBlock *dataBlock, u_int64_t *idx);

// Removes item at position idx.
void DataBlock_DeleteItem(DataBlock *dataBlock, u_int64_t idx);

// Free block.
void DataBlock_Free(DataBlock *block);

#endif
