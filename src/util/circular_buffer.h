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
typedef void(*CircularBufferItemFree)(void *);

CircularBuffer CircularBuffer_New
(
	size_t item_size,               // size of item in bytes
	uint cap,                       // max number of items in buffer
	CircularBufferItemFree free_cb  // [optional] item delete callback
);

// returns the element at the i'th position
void *CircularBuffer_GetElement
(
	CircularBuffer cb,  // buffer
	uint idx            // index of wanted element
);

// adds an item to buffer
// returns 1 on success, 0 otherwise
int CircularBuffer_Add
(
	CircularBuffer cb,  // buffer to populate
	void *item          // item to add
);

// forcefully adds item to buffer
// in case buffer is full an element is overwritten
void CircularBuffer_AddForce
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

// traverse
void CircularBuffer_TraverseCB
(
	CircularBuffer cb,           // buffer
	CircularBuffer_cb callback,  // callback to call on traversed elements
	uint start_idx,              // start index
	uint n                       // amount of elements to traverse
);

// returns number of items in buffer
int CircularBuffer_ItemCount
(
	CircularBuffer cb  // buffer to inspect
);

// set the read pointer to a wanted index relative to the write pointer (before)
void CircularBuffer_SetReadBehindWrite
(
	CircularBuffer cb,  // buffer
	uint cnt            // the amount of indexes behind the write pointer
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

// free buffer (does not free its elements if its free callback is NULL)
void CircularBuffer_Free
(
	CircularBuffer cb  // buffer to free
);
