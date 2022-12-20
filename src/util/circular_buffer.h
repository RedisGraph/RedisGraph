/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// forward declaration
typedef struct _CircularBuffer _CircularBuffer;
typedef _CircularBuffer* CircularBuffer;

typedef void (*CircularBuffer_ViewAllCallback)(void *user_data, const void *item);
typedef void (*CircularBuffer_ElementFree)(void *user_data, void *item);

CircularBuffer CircularBuffer_New
(
	const uint64_t item_size,
	const uint64_t capacity
);

void CircularBuffer_SetDeleter
(
	CircularBuffer cb,
	CircularBuffer_ElementFree deleter,
	void *user_data
);

uint64_t CircularBuffer_GetCapacity(const CircularBuffer circular_buffer);
// (Re)sets the capacity of the circular buffer. If the new capacity is less
// than what it has been, all the elements past the new capacity are gone.
// Returns true if the capacity setting was successfull, false otherwise.
bool CircularBuffer_SetCapacity(CircularBuffer *cb_ptr, const uint64_t new_capacity);

// Adds an item to the buffer, advances the write offset, and, in case of
// a full circle, the read offset as well.
void CircularBuffer_Add
(
	CircularBuffer circular_buffer,
	const void *item
);

// Removes the oldest item from buffer. If there was nothing to read, and so,
// remove, the item pointer is not changed.
void CircularBuffer_Read
(
	CircularBuffer cb,  // buffer to remove item from
	void *item          // [output] pointer populated with removed item
);

// Iterates over the entire unread contents and invoke the callback.
// Does it without making copies (memcpy) but rather passing a view pointer to
// the callback function.
void CircularBuffer_ViewAll
(
	CircularBuffer buffer,
	CircularBuffer_ViewAllCallback callback,
	void *user_data
);

// Returns a pointer to the current item without adjusting the read offset.
void *CircularBuffer_ViewCurrent
(
	const CircularBuffer cb
);

// Returns number of items in buffer.
uint64_t CircularBuffer_ItemCount
(
	const CircularBuffer cb  // buffer to inspect
);

uint64_t CircularBuffer_ItemSize
(
	const CircularBuffer cb
);

// Returns true if buffer there is nothing to read.
bool CircularBuffer_IsEmpty
(
	const CircularBuffer cb  // buffer to inspect
);

void CircularBuffer_Free
(
	CircularBuffer cb
);
