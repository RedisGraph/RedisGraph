/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _DATABLOCK_ITERATOR_H_
#define _DATABLOCK_ITERATOR_H_

#include "./block.h"
#include <sys/types.h>

/* Datablock iterator iterates over items within a datablock. */

typedef struct DataBlockIterator {
    Block *_start_block;    // Current block.
    Block *_current_block;
    int _start_pos;             // Iterator initial position.
    int _current_pos;           // Iterator current position.
    int _block_pos;             // Position within a block.
    int _end_pos;               // Iterator won't pass end position.
    int _step;                  // Increase current_pos by step each iteration.
} DataBlockIterator;

// Creates a new datablock iterator.
DataBlockIterator *DataBlockIterator_New (
    Block *block,       // Block from which iteration begins.
    int64_t start_pos,      // Iteration starts here.
    int64_t end_pos,        // Iteration stops here.
    int step            // To scan entire range, set step to 1.
);

#define DataBlockIterator_Position(iter) (iter)->_current_pos

// Clones given iterator.
DataBlockIterator *DataBlockIterator_Clone (
    const DataBlockIterator *it  // Iterator to clone.
);

// Returns the next item, unless we've reached the end
// in which case NULL is returned.
void *DataBlockIterator_Next(DataBlockIterator *iter);

// Reset iterator to original position.
void DataBlockIterator_Reset(DataBlockIterator *iter);

// Free iterator.
void DataBlockIterator_Free(DataBlockIterator *iter);

#endif
