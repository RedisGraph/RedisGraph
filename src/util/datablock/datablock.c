/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "datablock.h"
#include "datablock_iterator.h"

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

Block *_Block_New(size_t itemSize) {
    assert(itemSize > 0);
    Block *block = calloc(1, sizeof(Block) + BLOCK_CAP * itemSize);
    block->itemSize = itemSize;
    return block;
}

void _Block_Free(Block *block) {
    assert(block);
    free(block);
}

//------------------------------------------------------------------------------
// DataBlock
//------------------------------------------------------------------------------

void _DataBlock_AddBlocks(DataBlock *dataBlock, size_t blockCount) {
    assert(dataBlock && blockCount > 0);

    size_t prevBlockCount = dataBlock->blockCount;
    dataBlock->blockCount += blockCount;
    if(!dataBlock->blocks)
        dataBlock->blocks = malloc(sizeof(Block*) * dataBlock->blockCount);
    else
        dataBlock->blocks = realloc(dataBlock->blocks, sizeof(Block*) * dataBlock->blockCount);

    int i;
    for(i = prevBlockCount; i < dataBlock->blockCount; i++) {
        dataBlock->blocks[i] = _Block_New(dataBlock->itemSize);
        if(i>0) dataBlock->blocks[i-1]->next = dataBlock->blocks[i];
    }
    dataBlock->blocks[i-1]->next = NULL;

    dataBlock->itemCap = dataBlock->blockCount * BLOCK_CAP;
}

DataBlock *DataBlock_New(size_t itemCap, size_t itemSize) {
    DataBlock *dataBlock = malloc(sizeof(DataBlock));    
    dataBlock->itemCount = 0;
    dataBlock->itemSize = itemSize;
    dataBlock->blockCount = 0;
    dataBlock->blocks = NULL;
    _DataBlock_AddBlocks(dataBlock, ITEM_COUNT_TO_BLOCK_COUNT(itemCap));
    return dataBlock;
}

DataBlockIterator *DataBlock_Scan(const DataBlock *dataBlock) {
    assert(dataBlock);
    Block *startBlock = dataBlock->blocks[0];
    return DataBlockIterator_New(startBlock, 0, dataBlock->itemCount, 1);
}

void *DataBlock_GetItem(const DataBlock *dataBlock, size_t idx) {
    assert(dataBlock && idx >= 0 && idx < dataBlock->itemCount);
    Block *block = GET_ITEM_BLOCK(dataBlock, idx);
    idx = ITEM_POSITION_WITHIN_BLOCK(idx);
    return block->data + (idx * block->itemSize);
}

void DataBlock_SetItem(DataBlock *dataBlock, void *item, size_t idx) {
    assert(dataBlock && idx <= dataBlock->itemCount);

    // Get block.
    size_t blockIdx = ITEM_INDEX_TO_BLOCK_INDEX(idx);
    Block *block = dataBlock->blocks[blockIdx];

    // Add item to block.
    idx = ITEM_POSITION_WITHIN_BLOCK(idx);
    size_t offset = idx * block->itemSize;
    memcpy(block->data + offset , item, dataBlock->itemSize);
}

void DataBlock_AddItems(DataBlock *dataBlock, size_t itemCount, DataBlockIterator **it) {
    assert(dataBlock && itemCount > 0);
    
    // Make sure we've got room for items.
    if(dataBlock->itemCount + itemCount >= dataBlock->itemCap) {
        // Allocate twice as much items then we currently hold.
        size_t newCap = (dataBlock->itemCount + itemCount) * 2;
        size_t requiredAdditionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(newCap) - dataBlock->blockCount;
       _DataBlock_AddBlocks(dataBlock, requiredAdditionalBlocks);
    }

    if(it) {
        int step = 1;
        Block *block = ACTIVE_BLOCK(dataBlock);
        *it = DataBlockIterator_New(block, dataBlock->itemCount,
                                    dataBlock->itemCount + itemCount, step);
    }

    dataBlock->itemCount += itemCount;
}

Block *DataBlock_GetItemBlock(const DataBlock *dataBlock, size_t itemIdx) {
    return GET_ITEM_BLOCK(dataBlock, itemIdx);
}

void DataBlock_MigrateItem(DataBlock *dataBlock, size_t src, size_t dest) {
    assert(dataBlock);

    // Get the block in which dest item resides.
    Block *destItemBlock = GET_ITEM_BLOCK(dataBlock, dest);
    
    // Get item position within its block.
    size_t destItemBlockIdx = ITEM_POSITION_WITHIN_BLOCK(dest);
    size_t destItemOffset = (destItemBlockIdx * dataBlock->itemSize);

    // Get the src item.
    void *srcItem = DataBlock_GetItem(dataBlock, src);

    // Replace dest node with src node.
    memcpy(destItemBlock->data + destItemOffset, srcItem, dataBlock->itemSize);

    dataBlock->itemCount--;
}

void DataBlock_FreeTop(DataBlock *dataBlock, size_t count) {
    assert(dataBlock && dataBlock->itemCount >= count);
    dataBlock->itemCount -= count;
}

void DataBlock_Free(DataBlock *dataBlock) {
    for(int i = 0; i < dataBlock->blockCount; i++)
        _Block_Free(dataBlock->blocks[i]);

    free(dataBlock->blocks);
    free(dataBlock);
}
