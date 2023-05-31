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

	CircularBuffer cb = rm_malloc(sizeof(_CircularBuffer) + item_size * cap);

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

// sets the read pointer to be 'n' entries behind the write pointer
void CircularBuffer_ResetReader
(
	CircularBuffer cb,  // circular buffer
	int n               // set read 'n' entries behind write
) {
	// set the read pointer to min(#items, n) items behind the write pointer
	n = MIN(n, CircularBuffer_ItemCount(cb));

	// compensate for circularity
	uint64_t write = cb->write;
	uint write_idx = write / cb->item_size;
	int sub = write_idx - n;
	if(sub >= 0) {
		cb->read = (cb->data + write) - (n * cb->item_size);
	} else {
		cb->read = cb->end_marker + (sub * cb->item_size);
	}
}

// adds an item to buffer
// returns 1 on success, 0 otherwise
int CircularBuffer_Add
(
	CircularBuffer cb,  // buffer to populate
	void *item           // item to add
) {
	ASSERT(cb != NULL);
	ASSERT(item != NULL);

	// do not add item if buffer is full
	if(unlikely(CircularBuffer_Full(cb))) {
		return 0;
	}

	// copy item into buffer
	uint64_t write = cb->write;
	memcpy(cb->data + write, item, cb->item_size);

	// atomic update buffer item count
	cb->item_count++;

	// advance write position
	// circle back if write reached the end of the buffer
	write += cb->item_size;
	if(unlikely(cb->data + write >= cb->end_marker)) {
		write = 0;
	}

	// update write
	cb->write = write;

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
	uint64_t curr = cb->write;
	uint64_t next = curr + cb->item_size;
	if(unlikely(cb->data + next >= cb->end_marker)) {
		next = 0;
	}

	// advance write position atomicly
	while(!atomic_compare_exchange_weak(&cb->write, &curr, next)) {
		curr = cb->write;
		next = curr + cb->item_size;
		if(unlikely(cb->data + next >= cb->end_marker)) {
			next = 0;
		}
	}

	// increase and cap item count
	cb->item_count++;
	cb->item_count = MIN(cb->item_count, cb->item_cap);

	return cb->data + curr;
}

// removes oldest item from buffer
bool CircularBuffer_Remove
(
	CircularBuffer cb,  // buffer to remove item from
	void *item          // [output] pointer populated with removed item
) {
	ASSERT(cb != NULL);
	ASSERT(item != NULL);

	// make sure there's data to return
	if(unlikely(CircularBuffer_Empty(cb))) {
		return false;
	}

	// update buffer item count
	cb->item_count--;

	// copy item from buffer to output
	memcpy(item, cb->read, cb->item_size);

	// advance read position
	// circle back if read reached the end of the buffer
	cb->read += cb->item_size;
	if(unlikely(cb->read >= cb->end_marker)) {
		cb->read = cb->data;
	}

	// report success
	return true;
}

// read oldest item from buffer
bool CircularBuffer_Read
(
	CircularBuffer cb,  // buffer to read item from
	void *item          // [output] pointer populated with removed item
) {
	ASSERT(cb != NULL);
	ASSERT(item != NULL);

	bool res = CircularBuffer_Remove(cb, item);

	// compensate CircularBuffer_Remove reduction of item_count
	if(res != 0) {
		// restore item_count
		cb->item_count++;
	}

	return res;
}

// free buffer (does not free its elements if its free callback is NULL)
void CircularBuffer_Free
(
	CircularBuffer cb  // buffer to free
) {
	ASSERT(cb != NULL);

	rm_free(cb);
}

