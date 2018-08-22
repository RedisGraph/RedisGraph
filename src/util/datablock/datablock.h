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

/* Data block is a type agnostic continues block of memory 
 * used to hold items of the same type, each block has a next 
 * pointer to another block or NULL if this is the last block. */

typedef struct DataBlock {
    size_t itemCount;       // Number of items stored in datablock.
    size_t itemCap;         // Number of items datablock can hold.
    size_t blockCount;      // Number of blocks in datablock.
    size_t itemSize;        // Size of a single Item in bytes.
    Block **blocks;         // Array of blocks.
} DataBlock;

// Create a new DataBlock
// itemCap - number of items datablock can hold before resizing.
// itemSize - item size in bytes.
DataBlock *DataBlock_New(size_t itemCap, size_t itemSize);

// Returns an iterator which scans entire datablock.
DataBlockIterator *DataBlock_Scan(const DataBlock *dataBlock);

// Get item at position idx
void *DataBlock_GetItem(const DataBlock *dataBlock, size_t idx);

// Set item at position idx.
void DataBlock_SetItem(DataBlock *dataBlock, void *item, size_t idx);

void DataBlock_AddItems(DataBlock *dataBlock, size_t itemCount, DataBlockIterator **it);

Block *DataBlock_GetItemBlock(const DataBlock *dataBlock, size_t itemIdx);

// Overides item at index dest with item at index src
// Reduces datablock item count by 1.
void DataBlock_MigrateItem(DataBlock *dataBlock, size_t src, size_t dest);

// Free count last elements.
void DataBlock_FreeTop(DataBlock *dataBlock, size_t count);

// Free block.
void DataBlock_Free(DataBlock *block);

#endif
