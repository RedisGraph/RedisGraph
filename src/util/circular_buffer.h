/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdbool.h>

// forward declaration
typedef struct _CircularBuffer _CircularBuffer;
typedef _CircularBuffer* CircularBuffer;

CircularBuffer CircularBuffer_New
(
	size_t item_size,  // size of item in bytes
	uint cap           // max number of items in buffer
);

// adds an item to buffer
// returns 1 on success, 0 otherwise
int CircularBuffer_Add
(
	CircularBuffer cb,  // buffer to populate
	void *item          // item to add
);

// removes oldest item from buffer
// returns 1 on success, 0 otherwise
int CircularBuffer_Remove
(
	CircularBuffer cb,  // buffer to remove item from
	void *item          // [output] pointer populated with removed item
);

void *CircularBuffer_Current
(
	const CircularBuffer cb
);

void CircularBuffer_Advance
(
	CircularBuffer cb
);

// returns number of items in buffer
int CircularBuffer_ItemCount
(
	CircularBuffer cb  // buffer to inspect
);

// return true if buffer is empty
bool CircularBuffer_Empty
(
	const CircularBuffer cb  // buffer to inspect
);

// returns true if buffer is full
bool CircularBuffer_Full
(
	const CircularBuffer cb  // buffer to inspect
);

// free buffer
void CircularBuffer_Free
(
	CircularBuffer *cb  // buffer to free
);

