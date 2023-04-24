/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdbool.h>
#include <sys/types.h>

// forward declaration
typedef struct _CircularBuffer _CircularBuffer;
typedef _CircularBuffer* CircularBuffer;
typedef void(*CircularBufferItemFree)(void *);
typedef void (*CircularBuffer_ReadCallback)(const void *item, void *user_data);

CircularBuffer CircularBuffer_New
(
	size_t item_size,               // size of item in bytes
	uint cap,                       // max number of items in buffer
	CircularBufferItemFree free_cb  // [optional] item delete callback
);

// returns number of items in buffer
int CircularBuffer_ItemCount
(
	CircularBuffer cb  // buffer to inspect
);

uint CircularBuffer_ItemSize
(
	const CircularBuffer cb  // buffer
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

void *CircularBuffer_Current
(
	const CircularBuffer cb
);

// traverse circular buffer cb, from n items before the last written item.
// if n > # items in cb --> all items will be visited once
// note: this function sets the read pointer
void CircularBuffer_TraverseCBFromLast
(
	const CircularBuffer cb,               // buffer
	uint n,                                // # of items to traverse
	CircularBuffer_ReadCallback callback,  // callback called on elements
	void *user_data                        // additional data for the callback
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

// free buffer (does not free its elements if its free callback is NULL)
void CircularBuffer_Free
(
	CircularBuffer cb  // buffer to free
);
