/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "datablock_iterator.h"
#include "RG.h"
#include "datablock.h"
#include "../rmalloc.h"
#include <stdio.h>
#include <stdbool.h>

DataBlockIterator *DataBlockIterator_New(Block *block, uint64_t start_pos, uint64_t end_pos, uint step) {
	ASSERT(block && end_pos >= start_pos && step >= 1);

	DataBlockIterator *iter = rm_malloc(sizeof(DataBlockIterator));
	iter->_start_block = block;
	iter->_current_block = block;
	iter->_block_pos = start_pos % DATABLOCK_BLOCK_CAP;
	iter->_start_pos = start_pos;
	iter->_current_pos = iter->_start_pos;
	iter->_end_pos = end_pos;
	iter->_step = step;
	return iter;
}

DataBlockIterator *DataBlockIterator_Clone(const DataBlockIterator *it) {
	return DataBlockIterator_New(it->_start_block, it->_start_pos, it->_end_pos, it->_step);
}

void *DataBlockIterator_Next(DataBlockIterator *iter, uint64_t *id) {
	ASSERT(iter != NULL);

	// Set default.
	void *item = NULL;
	DataBlockItemHeader *item_header = NULL;

	// Have we reached the end of our iterator?
	while(iter->_current_pos < iter->_end_pos && iter->_current_block != NULL) {
		// Get item at current position.
		Block *block = iter->_current_block;
		item_header = (DataBlockItemHeader *)block->data + (iter->_block_pos * block->itemSize);

		// Advance to next position.
		iter->_block_pos += iter->_step;
		iter->_current_pos += iter->_step;

		// Advance to next block if current block consumed.
		if(iter->_block_pos >= DATABLOCK_BLOCK_CAP) {
			iter->_block_pos -= DATABLOCK_BLOCK_CAP;
			iter->_current_block = iter->_current_block->next;
		}

		if(IS_ITEM_INITIALIZED(item_header)) {
			item = ITEM_DATA(item_header);
			if(id) *id = iter->_current_pos - iter->_step;
			break;
		}
	}

	return item;
}

void DataBlockIterator_Reset(DataBlockIterator *iter) {
	ASSERT(iter != NULL);
	iter->_block_pos = iter->_start_pos % DATABLOCK_BLOCK_CAP;
	iter->_current_block = iter->_start_block;
	iter->_current_pos = iter->_start_pos;
}

void DataBlockIterator_Free(DataBlockIterator *iter) {
	ASSERT(iter != NULL);
	rm_free(iter);
}

