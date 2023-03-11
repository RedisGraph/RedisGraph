/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "datablock_iterator.h"
#include "RG.h"
#include "datablock.h"
#include "../rmalloc.h"
#include <stdio.h>
#include <stdbool.h>

DataBlockIterator *DataBlockIterator_New
(
	Block **blocks,
	uint64_t block_cap,
	int64_t start_pos,
	int64_t end_pos
) {
	ASSERT(blocks != NULL);
	ASSERT(block_cap > 0);

	DataBlockIterator *iter = rm_malloc(sizeof(DataBlockIterator));

	iter->step          = (end_pos >= start_pos) ? 1 : -1;
	iter->blocks        = blocks;
	iter->end_pos       = end_pos;
	iter->start_pos     = start_pos;
	iter->block_cap     = block_cap;
	iter->block_pos     = ITEM_POSITION_WITHIN_BLOCK(start_pos, block_cap);
	iter->current_pos   = start_pos;
	iter->current_block = blocks[ITEM_IDX_TO_BLOCK_IDX(start_pos, block_cap)];

	return iter;
}

void *DataBlockIterator_Next
(
	DataBlockIterator *it,
	uint64_t *id
) {
	ASSERT(it != NULL);

	// set default
	void                *item        = NULL;
	DataBlockItemHeader *item_header = NULL;

	// have we reached the end of our iterator?
	while(it->current_pos != it->end_pos) {
		// get item at current position
		const Block *block = it->current_block;
		item_header = (DataBlockItemHeader*)(block->data +
			(it->block_pos * block->itemSize));

		// advance to next position
		it->block_pos   += it->step;
		it->current_pos += it->step;

		// advance to next block if current block is consumed
		// and iterator is within range
		if((it->block_pos == it->block_cap || it->block_pos == -1) &&
		   (it->current_pos != it->end_pos)) {
			// compute position within block
			int64_t block_pos =
				ITEM_POSITION_WITHIN_BLOCK(it->current_pos, it->block_cap);
			// compute block index
			int64_t block_idx =
				ITEM_IDX_TO_BLOCK_IDX(it->current_pos, it->block_cap);

			// update iterator
			it->block_pos     = block_pos;
			it->current_block = it->blocks[block_idx];
		}

		if(!IS_ITEM_DELETED(item_header)) {
			item = ITEM_DATA(item_header);
			if(id != NULL) {
				*id = it->current_pos - it->step;
			}
			break;
		}
	}

	return item;
}

void DataBlockIterator_Reset
(
	DataBlockIterator *iter
) {
	ASSERT(iter != NULL);

	int64_t s = iter->start_pos;
	int64_t cur_block_idx = ITEM_IDX_TO_BLOCK_IDX(s, iter->block_cap);

	iter->block_pos     = ITEM_POSITION_WITHIN_BLOCK(s, iter->block_cap);
	iter->current_pos   = s;
	iter->current_block = iter->blocks[cur_block_idx];
}

void DataBlockIterator_Free
(
	DataBlockIterator *iter
) {
	ASSERT(iter != NULL);
	rm_free(iter);
}

