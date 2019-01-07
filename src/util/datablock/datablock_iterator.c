/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "datablock_iterator.h"
#include "datablock.h"
#include "assert.h"
#include "../rmalloc.h"
#include <stdio.h>

int static inline _IsItemDeleted(int itemSize, unsigned char* item) {
    for(int i = 0; i < itemSize; i++) {
        if(item[i] != DELETED_MARKER) {
            return 0;
        }
    }
    return 1;
}

DataBlockIterator *DataBlockIterator_New(Block *block, int64_t start_pos, int64_t end_pos, int step) {
    assert(block && start_pos >= 0 && end_pos >= start_pos && step >= 1);
    
    DataBlockIterator *iter = rm_malloc(sizeof(DataBlockIterator));
    iter->_start_block = block;
    iter->_current_block = block;
    iter->_block_pos = start_pos % BLOCK_CAP;
    iter->_start_pos = start_pos;
    iter->_current_pos = iter->_start_pos;
    iter->_end_pos = end_pos;
    iter->_step = step;
    return iter;
}

DataBlockIterator *DataBlockIterator_Clone(const DataBlockIterator *it) {
    return DataBlockIterator_New(it->_start_block, it->_start_pos, it->_end_pos, it->_step);
}

void *DataBlockIterator_Next(DataBlockIterator *iter) {
    assert(iter);
    
    if(iter->_current_pos >= iter->_end_pos || iter->_current_block == NULL) return NULL;
    
    size_t itemSize = iter->_current_block->itemSize;
    unsigned char *item = NULL;
    // Have we reached the end of our iterator?
    while(iter->_current_pos < iter->_end_pos && iter->_current_block != NULL) {
        // Get item at current position.
        Block *block = iter->_current_block;
        item = block->data + (iter->_block_pos * block->itemSize);

        // Advance to next position.
        iter->_block_pos += iter->_step;
        iter->_current_pos += iter->_step;

        // Advance to next block if current block consumed.
        if(iter->_block_pos >= BLOCK_CAP) {
            iter->_block_pos -= BLOCK_CAP;
            iter->_current_block = iter->_current_block->next;
        }

        if(_IsItemDeleted(itemSize, item)) {
            item = NULL;
            continue;
        }

        break;
    }
    
    return (void*)item;
}

void DataBlockIterator_Reset(DataBlockIterator *iter) {
    assert(iter);
    iter->_block_pos = iter->_start_pos % BLOCK_CAP;
    iter->_current_block = iter->_start_block;
    iter->_current_pos = iter->_start_pos;
}

void DataBlockIterator_Free(DataBlockIterator *iter) {
    assert(iter);
    rm_free(iter);
}
