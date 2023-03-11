/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdint.h>
#include "../block.h"

/* Datablock iterator iterates over items within a datablock. */

typedef struct {
	Block **blocks;              // blocks to iterate over
	const Block *current_block;  // current block
	int64_t block_pos;           // position within a block
	int64_t current_pos;         // iterator global position
	int64_t block_cap;           // max number of items in block
	int64_t start_pos;           // iterator start position
	int64_t end_pos;             // iterator won't pass end position
	int8_t step;                 // advance direction forward/backwards
} DataBlockIterator;

// creates a new datablock iterator
DataBlockIterator *DataBlockIterator_New
(
	Block **blocks,      // blocks to iterate over
	uint64_t block_cap,  // block capacity
	int64_t start_pos,   // iteration starts here
	int64_t end_pos      // iteration stops here
);

#define DataBlockIterator_Position(iter) (iter)->_current_pos

// Returns the next item, unless we've reached the end
// in which case NULL is returned.
// if `id` is provided and an item is located
// `id` will be set to the returned item index
void *DataBlockIterator_Next(DataBlockIterator *iter, uint64_t *id);

// Reset iterator to original position.
void DataBlockIterator_Reset(DataBlockIterator *iter);

// Free iterator.
void DataBlockIterator_Free(DataBlockIterator *iter);

