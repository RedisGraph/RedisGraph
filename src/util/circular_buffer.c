/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */


#include "RG.h"
#include "rmalloc.h"
#include "circular_buffer.h"
#include "graph/graphcontext.h"

#include <stdatomic.h>

// circular buffer structure
// the buffer is of fixed size
// items are removed by order of insertion, similar to a queue
struct _CircularBuffer {
	char *read;                   // read data from here
	_Atomic uint64_t write;       // write data to here
	size_t item_size;             // item size in bytes
	_Atomic uint64_t item_count;  // current number of items in buffer
	uint64_t item_cap;            // max number of items held by buffer
	char *end_marker;             // marks the end of the buffer
	char data[];                  // data
};

CircularBuffer CircularBuffer_New
(
	size_t item_size,  // size of item in bytes
	uint cap           // max number of items in buffer
) {
	CircularBuffer cb = rm_calloc(1, sizeof(_CircularBuffer) + item_size * cap);

	cb->read       = cb->data;   // read from beginning of data
	cb->write      = 0;          // write to beginning of data
	cb->item_cap   = cap;        // buffer capacity
	cb->item_size  = item_size;  // item size
	cb->item_count = 0;          // no items in buffer
	cb->end_marker = cb->data + (item_size * cap);

	return cb;
}

// returns number of items in buffer
uint64_t CircularBuffer_ItemCount
(
	CircularBuffer cb  // buffer to inspect
) {
	ASSERT(cb != NULL);

	return cb->item_count;
}

// returns buffer capacity
uint64_t CircularBuffer_Cap
(
	CircularBuffer cb // buffer
) {
	ASSERT(cb != NULL);

	return cb->item_cap;
}

uint CircularBuffer_ItemSize
(
	const CircularBuffer cb  // buffer
) {
	return cb->item_size;
}

// return true if buffer is empty
inline bool CircularBuffer_Empty
(
	const CircularBuffer cb  // buffer to inspect
) {
	ASSERT(cb != NULL);

	return cb->item_count == 0;
}

// returns true if buffer is full
inline bool CircularBuffer_Full
(
	const CircularBuffer cb  // buffer to inspect
) {
	ASSERT(cb != NULL);

	return cb->item_count == cb->item_cap;
}

// sets the read pointer to the beginning of the buffer
void CircularBuffer_ResetReader
(
	CircularBuffer cb  // circular buffer
) {
	// compensate for circularity
	uint64_t write = cb->write;
	uint write_idx = write / cb->item_size;
	int sub = write_idx - cb->item_count;
	if(sub >= 0) {
		cb->read = (cb->data + write) - (cb->item_count * cb->item_size);
	} else {
		cb->read = cb->end_marker + (sub * cb->item_size);
	}
}

// adds an item to buffer
// returns 1 on success, 0 otherwise
int CircularBuffer_Add
(
	CircularBuffer cb,   // buffer to populate
	void *item           // item to add
) {
	ASSERT(cb != NULL);
	ASSERT(item != NULL);

	// do not add item if buffer is full
	if(unlikely(CircularBuffer_Full(cb))) {
		return 0;
	}

	uint64_t write = atomic_fetch_add(&cb->write, cb->item_size);
	if(unlikely(cb->data + write >= cb->end_marker)) {
		cb->write = 0;
		return 0;
	}

	// atomic update buffer item count
	cb->item_count++;

	// copy item into buffer
	memcpy(cb->data + write, item, cb->item_size);

	// report success
	return 1;
}

// reserve a slot within buffer
// returns a pointer to a 'item size' slot within the buffer
// this function is thread-safe and lock-free
void *CircularBuffer_Reserve
(
	CircularBuffer cb  // buffer to populate
) {
	ASSERT(cb != NULL);

	// determine current and next write position
	uint64_t curr = atomic_fetch_add(&cb->write, cb->item_size);
	if(unlikely(cb->data + curr >= cb->end_marker)) {
		uint64_t old_curr = curr + cb->item_size;
		curr -= cb->item_size * cb->item_cap;
		// advance write position atomicly
		atomic_compare_exchange_weak(&cb->write, &old_curr, curr);
	} else {
		cb->item_count++;
	}

	return cb->data + curr;
}

// read oldest item from buffer
void *CircularBuffer_Read
(
	CircularBuffer cb,  // buffer to read item from
	void *item          // [optional] pointer populated with removed item
) {
	ASSERT(cb != NULL);

	// make sure there's data to return
	if(unlikely(CircularBuffer_Empty(cb))) {
		return NULL;
	}

	void *read = cb->read;

	// update buffer item count
	cb->item_count--;

	// copy item from buffer to output
	if(item != NULL) {
		memcpy(item, cb->read, cb->item_size);
	}

	// advance read position
	// circle back if read reached the end of the buffer
	cb->read += cb->item_size;
	if(unlikely(cb->read >= cb->end_marker)) {
		cb->read = cb->data;
	}

	// return original read position
	return read;
}

// free buffer (does not free its elements if its free callback is NULL)
void CircularBuffer_Free
(
	CircularBuffer cb  // buffer to free
) {
	ASSERT(cb != NULL);

	rm_free(cb);
}

