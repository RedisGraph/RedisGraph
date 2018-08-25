/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "datablock_iterator.h"
#include "assert.h"
#include <stdio.h>

DataBlockIterator *DataBlockIterator_New(Block *block, int start_pos, int end_pos, int step) {
    assert(block && start_pos >= 0 && end_pos >= start_pos && step >= 1);
    
    DataBlockIterator *iter = malloc(sizeof(DataBlockIterator));
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
    // Have we reached the end of our iterator?
    if(iter->_current_pos >= iter->_end_pos || iter->_current_block == NULL) return NULL;
    
    // Get item at current position.
    Block *block = iter->_current_block;
    void *item = block->data + (iter->_block_pos * block->itemSize);

    // Advance to next position.
    iter->_block_pos += iter->_step;
    iter->_current_pos += iter->_step;

    // Advance to next block if current block consumed.
    if(iter->_block_pos >= BLOCK_CAP) {
        iter->_block_pos -= BLOCK_CAP;
        iter->_current_block = iter->_current_block->next;
    }

    return item;
}

void DataBlockIterator_Reset(DataBlockIterator *iter) {
    assert(iter);
    iter->_block_pos = iter->_start_pos % BLOCK_CAP;
    iter->_current_block = iter->_start_block;
    iter->_current_pos = iter->_start_pos;
}

void DataBlockIterator_Free(DataBlockIterator *iter) {
    assert(iter);
    free(iter);
}
