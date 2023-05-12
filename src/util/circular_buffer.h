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

CircularBuffer CircularBuffer_New
(
	size_t item_size,               // size of item in bytes
	uint cap,                       // max number of items in buffer
	CircularBufferItemFree free_cb  // [optional] item delete callback
);

// returns number of items in buffer
uint64_t CircularBuffer_ItemCount
(
	CircularBuffer cb  // buffer
);

// returns buffer capacity
uint64_t CircularBuffer_Cap
(
	CircularBuffer cb // buffer
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
bool CircularBuffer_Remove
(
	CircularBuffer cb,  // buffer to remove item from
	void *item          // [output] pointer populated with removed item
);

// read oldest item from buffer
bool CircularBuffer_Read
(
	CircularBuffer cb,  // buffer to read item from
	void *item          // [output] pointer populated with removed item
);

// sets the read pointer to be 'n' entries behind the write pointer
void CircularBuffer_ResetReader
(
	CircularBuffer cb,  // circular buffer
	int n               // set read 'n' entries behind write
);

// free buffer (does not free its elements if its free callback is NULL)
void CircularBuffer_Free
(
	CircularBuffer cb  // buffer to free
);

