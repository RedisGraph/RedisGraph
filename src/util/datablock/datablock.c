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
#include "datablock.h"
#include "datablock_iterator.h"
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

Block *_Block_New(size_t itemSize) {
    assert(itemSize > 0);
    Block *block = rm_calloc(1, sizeof(Block) + BLOCK_CAP * itemSize);
    block->itemSize = itemSize;
    return block;
}

void _Block_Free(Block *block) {
    assert(block);
    rm_free(block);
}

//------------------------------------------------------------------------------
// DataBlock
//------------------------------------------------------------------------------

void _DataBlock_AddBlocks(DataBlock *dataBlock, size_t blockCount) {
    assert(dataBlock && blockCount > 0);

    size_t prevBlockCount = dataBlock->blockCount;
    dataBlock->blockCount += blockCount;
    if(!dataBlock->blocks)
        dataBlock->blocks = rm_malloc(sizeof(Block*) * dataBlock->blockCount);
    else
        dataBlock->blocks = rm_realloc(dataBlock->blocks, sizeof(Block*) * dataBlock->blockCount);

    int i;
    for(i = prevBlockCount; i < dataBlock->blockCount; i++) {
        dataBlock->blocks[i] = _Block_New(dataBlock->itemSize);
        if(i>0) dataBlock->blocks[i-1]->next = dataBlock->blocks[i];
    }
    dataBlock->blocks[i-1]->next = NULL;

    dataBlock->itemCap = dataBlock->blockCount * BLOCK_CAP;
}

void static inline _DataBlock_MarkItemAsDeleted(const DataBlock *dataBlock, unsigned char* item) {
    memset(item, DELETED_MARKER, dataBlock->itemSize);
}

void static inline _DataBlock_MarkItemAsUndelete(const DataBlock *dataBlock, unsigned char* item) {
    item[0] = !DELETED_MARKER;
}

int static inline _DataBlock_IsItemDeleted(const DataBlock *dataBlock, unsigned char* item) {
    for(int i = 0; i < dataBlock->itemSize; i++) {
        if(item[i] != DELETED_MARKER) {
            return 0;
        }
    }
    return 1;
}

// Checks to see if idx is within global array bounds
// array bounds are between 0 and itemCount + #deleted indices
// e.g. [3, 7, 2, D, 1, D, 5] where itemCount = 5 and #deleted indices is 2
// and so it is valid to query the array with idx 6.
int static inline _DataBlock_IndexOutOfBounds(const DataBlock *dataBlock, size_t idx) {
    return (idx >= (dataBlock->itemCount + array_len(dataBlock->deletedIdx)));
}

DataBlock *DataBlock_New(size_t itemCap, size_t itemSize) {
    DataBlock *dataBlock = rm_malloc(sizeof(DataBlock));
    dataBlock->itemCount = 0;
    dataBlock->itemSize = itemSize;
    dataBlock->blockCount = 0;
    dataBlock->blocks = NULL;
    dataBlock->deletedIdx = array_new(uint64_t, 128);
    _DataBlock_AddBlocks(dataBlock, ITEM_COUNT_TO_BLOCK_COUNT(itemCap));
    return dataBlock;
}

DataBlockIterator *DataBlock_Scan(const DataBlock *dataBlock) {
    assert(dataBlock);
    Block *startBlock = dataBlock->blocks[0];

    int64_t startPos = 0;
    // Deleted items are skipped, we're about to perform 
    // array_len(dataBlock->deletedIdx) skips during out scan.
    int64_t endPos = dataBlock->itemCount + array_len(dataBlock->deletedIdx);
    return DataBlockIterator_New(startBlock, 0, endPos, 1);
}

// Make sure datablock can accommodate at least k items.
void DataBlock_Accommodate(DataBlock *dataBlock, int64_t k) {
    // Compute number of free slots.
    int64_t freeSlotsCount = dataBlock->itemCap - dataBlock->itemCount;
    int64_t additionalItems = k - freeSlotsCount;

    if(additionalItems > 0) {
        int64_t additionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(additionalItems);
        _DataBlock_AddBlocks(dataBlock, additionalBlocks);
    }
}

void *DataBlock_GetItem(const DataBlock *dataBlock, size_t idx) {
    assert(dataBlock);

    if(_DataBlock_IndexOutOfBounds(dataBlock, idx)) return NULL;

    Block *block = GET_ITEM_BLOCK(dataBlock, idx);
    idx = ITEM_POSITION_WITHIN_BLOCK(idx);
    
    unsigned char *item = block->data + (idx * block->itemSize);

    // Incase item is marked as deleted, return NULL.
    if(_DataBlock_IsItemDeleted(dataBlock, item)) return NULL;

    return item;
}

void* DataBlock_AllocateItem(DataBlock *dataBlock, uint64_t *idx) {
    // Make sure we've got room for items.
    if(dataBlock->itemCount >= dataBlock->itemCap) {
        // Allocate twice as much items then we currently hold.
        size_t newCap = dataBlock->itemCount * 2;
        size_t requiredAdditionalBlocks = ITEM_COUNT_TO_BLOCK_COUNT(newCap) - dataBlock->blockCount;
       _DataBlock_AddBlocks(dataBlock, requiredAdditionalBlocks);
    }

    // Get index into which to store item,
    // prefer reusing free indicies.
    uint64_t pos = dataBlock->itemCount;
    if(array_len(dataBlock->deletedIdx) > 0) {
        pos = array_pop(dataBlock->deletedIdx);
    }
    dataBlock->itemCount++;

    if(idx) *idx = pos;

    Block *block = GET_ITEM_BLOCK(dataBlock, pos);
    pos = ITEM_POSITION_WITHIN_BLOCK(pos);
    
    unsigned char *item = block->data + (pos * block->itemSize);
    _DataBlock_MarkItemAsUndelete(dataBlock, item);

    return (void*)item;
}

void DataBlock_DeleteItem(DataBlock *dataBlock, uint64_t idx) {
    assert(dataBlock);
    if(_DataBlock_IndexOutOfBounds(dataBlock, idx)) return;

    // Get block.
    size_t blockIdx = ITEM_INDEX_TO_BLOCK_INDEX(idx);
    Block *block = dataBlock->blocks[blockIdx];

    uint blockPos = ITEM_POSITION_WITHIN_BLOCK(idx);
    size_t offset = blockPos * block->itemSize;

    // Return if item already deleted.
    unsigned char *item = block->data + offset;
    if(_DataBlock_IsItemDeleted(dataBlock, item)) return;

    _DataBlock_MarkItemAsDeleted(dataBlock, item);
    dataBlock->deletedIdx = array_append(dataBlock->deletedIdx, idx);
    dataBlock->itemCount--;
}

void DataBlock_Free(DataBlock *dataBlock) {
    for(int i = 0; i < dataBlock->blockCount; i++)
        _Block_Free(dataBlock->blocks[i]);

    rm_free(dataBlock->blocks);
    array_free(dataBlock->deletedIdx);
    rm_free(dataBlock);
}
