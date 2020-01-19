/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "../block.h"

/* Datablock iterator iterates over items within a datablock. */

typedef struct {
	Block *_start_block;         // First block accessed by iterator.
	Block *_current_block;       // Current block.
	uint _start_pos;             // Iterator initial position.
	uint _current_pos;           // Iterator current position.
	uint _block_pos;             // Position within a block.
	uint _end_pos;               // Iterator won't pass end position.
	uint _step;                  // Increase current_pos by step each iteration.
} DataBlockIterator;

// Creates a new datablock iterator.
DataBlockIterator *DataBlockIterator_New(
	Block *block,       // Block from which iteration begins.
	uint start_pos,     // Iteration starts here.
	uint end_pos,       // Iteration stops here.
	uint step           // To scan entire range, set step to 1.
);

#define DataBlockIterator_Position(iter) (iter)->_current_pos

// Clones given iterator.
DataBlockIterator *DataBlockIterator_Clone(
	const DataBlockIterator *it  // Iterator to clone.
);

// Returns the next item, unless we've reached the end
// in which case NULL is returned.
void *DataBlockIterator_Next(DataBlockIterator *iter);

// Reset iterator to original position.
void DataBlockIterator_Reset(DataBlockIterator *iter);

// Free iterator.
void DataBlockIterator_Free(DataBlockIterator *iter);

