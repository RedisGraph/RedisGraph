/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */


#include "RG.h"
#include "rmalloc.h"
#include "circular_buffer.h"
#include "graph/graphcontext.h"

// circular buffer structure
// the buffer is of fixed size
// items are removed by order of insertion, similar to a queue
struct _CircularBuffer {
	char *read;                      // read data from here
	char *write;                     // write data to here
	size_t item_size;                // item size in bytes
	_Atomic int item_count;          // current number of items in buffer
	int item_cap;                    // max number of items held by buffer
	char *end_marker;                // marks the end of the buffer
	CircularBufferItemFree free_cb;  // free callback
	char data[];                     // data
};

CircularBuffer CircularBuffer_New
(
	size_t item_size,               // size of item in bytes
	uint cap,                       // max number of items in buffer
	CircularBufferItemFree free_cb  // [optional] item delete callback
) {
	CircularBuffer cb = rm_malloc(sizeof(_CircularBuffer) + item_size * cap);

	cb->read       = cb->data;   // read from beginning of data
	cb->write      = cb->data;   // write to beginning of data
	cb->item_cap   = cap;        // save cap
	cb->item_size  = item_size;  // save item size
	cb->item_count = 0;          // no items in buffer
	cb->end_marker = cb->data + (item_size * cap);
	cb->free_cb    = free_cb;

	return cb;
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
	memcpy(cb->write, item, cb->item_size);

	// atomic update buffer item count
	cb->item_count++;

	// advance write position
	// circle back if write reached the end of the buffer
	cb->write += cb->item_size;
	if(unlikely(cb->write >= cb->end_marker)) {
		cb->write = cb->data;
	}

	// report success
	return 1;
}

// forcefully adds item to buffer, i.e., may run over an existing element
// in case buffer is full an element is overwritten
void CircularBuffer_AddForce
(
	CircularBuffer cb,  // buffer to populate
	void *item          // item to add
) {
	ASSERT(cb != NULL);
	ASSERT(item != NULL);

	// copy item into buffer
	memcpy(cb->write, item, cb->item_size);

	// atomic update buffer item count
	if(!CircularBuffer_Full(cb)) {
		cb->item_count++;
	}

	// advance write position
	// circle back if write reached the end of the buffer
	cb->write += cb->item_size;
	if(unlikely(cb->write >= cb->end_marker)) {
		cb->write = cb->data;
	}
}

// removes oldest item from buffer
// returns 1 on success, 0 otherwise
int CircularBuffer_Remove
(
	CircularBuffer cb,  // buffer to remove item from
	void *item          // [output] pointer populated with removed item
) {
	ASSERT(cb != NULL);
	ASSERT(item != NULL);

	// make sure there's data to return
	if(unlikely(CircularBuffer_Empty(cb))) {
		return 0;
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
	return 1;
}

// returns a pointer to the element at the i'th position
void *CircularBuffer_GetElement
(
	CircularBuffer cb,  // buffer
	uint idx            // index of wanted element
) {
	ASSERT(cb != NULL);
	ASSERT(idx < cb->item_cap);

	return cb->data + idx * cb->item_size;
}

void *CircularBuffer_Current
(
	const CircularBuffer cb
) {
	ASSERT(cb != NULL);
	ASSERT(!CircularBuffer_Empty(cb));

	return cb->read;
}

// traverse circular buffer cb, from n items before the last written item.
// if n > # items in cb --> all items will be visited once
// note: this function sets the read pointer
void CircularBuffer_TraverseCBFromLast
(
	CircularBuffer cb,                     // buffer
	uint n,                                // # of items to traverse
	CircularBuffer_ReadCallback callback,  // callback called on elements
	void *user_data                        // additional data for the callback
) {
	// set the read pointer to min(#items, n) items behind the write pointer
	uint n_items = CircularBuffer_ItemCount(cb);
	uint n_items_back = MIN(n, n_items);

	// compensate for circularity
	uint write_ind = (cb->write - cb->data) / cb->item_size;
	int sub = write_ind - n_items_back;
	if(sub >= 0) {
		cb->read = cb->write - n_items_back * cb->item_size;
	} else {
		cb->read = cb->end_marker + (sub * cb->item_size);
	}

	// visit items
	while(n_items_back > 0) {
		// apply callback
		callback(user_data, cb->read);

		cb->read += cb->item_size;

		if(unlikely(cb->read >= cb->end_marker)) {
			cb->read = cb->data;
		}

		n_items_back--;
	}
}

// returns number of items in buffer
int CircularBuffer_ItemCount
(
	CircularBuffer cb  // buffer to inspect
) {
	ASSERT(cb != NULL);

	return cb->item_count;
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

// free buffer (does not free its elements if its free callback is NULL)
void CircularBuffer_Free
(
	CircularBuffer cb  // buffer to free
) {
	ASSERT(cb != NULL);

	if(cb->free_cb != NULL) {
		uint64_t read_offset = 0;
		while (read_offset < cb->item_count) {
			void **item = (void **)CircularBuffer_GetElement(cb, read_offset);
			cb->free_cb(*item);
			++read_offset;
		}
	}

	rm_free(cb);
}
