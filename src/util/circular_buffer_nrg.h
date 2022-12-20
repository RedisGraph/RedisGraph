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
typedef struct _CircularBufferNRG _CircularBufferNRG;
typedef _CircularBufferNRG* CircularBufferNRG;

typedef void (*CircularBufferNRG_ViewAllCallback)(void *user_data, const void *item);
typedef void (*CircularBufferNRG_ElementFree)(void *user_data, void *item);

CircularBufferNRG CircularBufferNRG_New
(
	const uint64_t item_size,
	const uint64_t capacity
);

void CircularBufferNRG_SetDeleter
(
	CircularBufferNRG cb,
	CircularBufferNRG_ElementFree deleter,
	void *user_data
);

uint64_t CircularBufferNRG_GetCapacity(const CircularBufferNRG circular_buffer);
// (Re)sets the capacity of the circular buffer. If the new capacity is less
// than what it has been, all the elements past the new capacity are gone.
// Returns true if the capacity setting was successfull, false otherwise.
bool CircularBufferNRG_SetCapacity(CircularBufferNRG *cb_ptr, const uint64_t new_capacity);

// Adds an item to the buffer, advances the write offset, and, in case of
// a full circle, the read offset as well.
void CircularBufferNRG_Add
(
	CircularBufferNRG circular_buffer,
	const void *item
);

// Removes the oldest item from buffer. If there was nothing to read, and so,
// remove, the item pointer is not changed.
void CircularBufferNRG_Read
(
	CircularBufferNRG cb,  // buffer to remove item from
	void *item             // [output] pointer populated with removed item
);

// Iterates over the entire unread contents and invoke the callback.
// Does it without making copies (memcpy) but rather passing a view pointer to
// the callback function.
void CircularBufferNRG_ViewAll
(
	CircularBufferNRG buffer,
	CircularBufferNRG_ViewAllCallback callback,
	void *user_data
);

// Returns a pointer to the current item without adjusting the read offset.
void *CircularBufferNRG_ViewCurrent
(
	const CircularBufferNRG cb
);

// Returns number of items in buffer.
uint64_t CircularBufferNRG_ItemCount
(
	const CircularBufferNRG cb  // buffer to inspect
);

uint64_t CircularBufferNRG_ItemSize
(
	const CircularBufferNRG cb
);

// Returns true if buffer there is nothing to read.
bool CircularBufferNRG_IsEmpty
(
	const CircularBufferNRG cb  // buffer to inspect
);

void CircularBufferNRG_Free
(
	CircularBufferNRG cb
);
