/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */
/*

This file contains a data structure of a circular buffer without a read
guarantee, meaning it is possible to overwrite the elements that haven't been
read yet. This is useful when we want to only be able to see the "last top-N"
elements stored within the container, and we aren't interested in the ones that
were added earlier than these "last top-N". Hence, this structure's name
contains the "NRG" suffix - "No Read Guarantee".

A few features of the data structure:

1. It is possible to specify a custom deleter specified if it is required for
certain types.
2. It can dynamically grow in size and shrink.
3. It allows two versions of accessing the stored elements - one that adjusts
the read pointer and one that doesn't; both are useful.
4. It is possible to traverse the whole collection without the overhead and
making copies, by providing a custom callback function which will be called for
each of the stored item, by providing a pointer to it for further
re-interpretation by the caller. During such a traversal, the custom callback
function may force the ongoing traversal to stop by means of the return value.

This circular buffer is not thread safe and requires external synchronisation.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct _CircularBufferNRG _CircularBufferNRG;
typedef _CircularBufferNRG* CircularBufferNRG;

// Should return true if the read has to be stopped right after the invocation.
typedef bool (*CircularBufferNRG_ReadAllCallback)(void *user_data, const void *item);
typedef void (*CircularBufferNRG_ElementFree)(void *user_data, void *item);
typedef void (*CircularBufferNRG_CloneItem)(const void *item_to_clone, void *destination_item, void *user_data);

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

void CircularBufferNRG_SetItemClone
(
    CircularBufferNRG cb,
    CircularBufferNRG_CloneItem clone,
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
    const void *item_to_add
);

// Removes the oldest item from buffer. If there was nothing to read, and so,
// remove, the item pointer is not changed.
void CircularBufferNRG_Read
(
    CircularBufferNRG cb,
    void *output
);

// Iterates over the entire unread contents and invokes the callback.
// Does it without making copies (memcpy) but rather passing a view pointer to
// the callback function, suitable for a reinterpret cast.
// This function adjusts the read offset.
void CircularBufferNRG_ReadAll
(
    CircularBufferNRG buffer,
    CircularBufferNRG_ReadAllCallback callback,
    void *user_data
);

// Iterates over the entire unread contents and invokes the callback.
// Does it without making copies (memcpy) but rather passing a view pointer to
// the callback function, suitable for a reinterpret cast.
// This doesn't adjust the read offset.
void CircularBufferNRG_ViewAll
(
    CircularBufferNRG buffer,
    CircularBufferNRG_ReadAllCallback callback,
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
    const CircularBufferNRG cb
);

uint64_t CircularBufferNRG_ItemSize
(
    const CircularBufferNRG cb
);

// Returns true if buffer there is nothing to read.
bool CircularBufferNRG_IsEmpty
(
    const CircularBufferNRG cb
);

void CircularBufferNRG_Free
(
    CircularBufferNRG cb
);
