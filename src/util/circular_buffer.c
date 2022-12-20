/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <RG.h>
#include "circular_buffer.h"

#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

typedef struct ElementDeleteInfo {
	CircularBuffer_ElementFree deleter;
	void *user_data;
} ElementDeleteInfo;

// A LIFO circular queue.
struct _CircularBuffer {
	uint64_t read_offset;              // read data from here
	uint64_t write_offset;             // write data to here
	uint64_t item_size;      // item size in bytes
	uint64_t item_count;
	bool is_circled;
	ElementDeleteInfo delete_info;
	char *end_marker;        // marks the end of the buffer
	char data[];
};

static void* _CircularBuffer_ItemAtOffset
(
	CircularBuffer cb,
	const uint64_t offset
) {
	ASSERT(cb);
	if (!cb || offset > cb->item_count) {
		return NULL;
	}
	return cb->data + offset * cb->item_size;
}

static void* _CircularBuffer_ItemAtReadOffset
(
	CircularBuffer cb
) {
	return _CircularBuffer_ItemAtOffset(cb, cb->read_offset);
}

static void* _CircularBuffer_ItemAtWriteOffset
(
	CircularBuffer cb
) {
	return _CircularBuffer_ItemAtOffset(cb, cb->write_offset);
}

static void _DeleteElement(
	void *item,
	ElementDeleteInfo info
) {
	if (!info.deleter) {
		return;
	}

	info.deleter(info.user_data, item);
}

CircularBuffer CircularBuffer_New
(
	const uint64_t item_size,
	const uint64_t capacity
) {
	ASSERT(item_size);
	ASSERT(capacity);
	if (!item_size || !capacity) {
		return NULL;
	}

	const uint64_t size_of_data = item_size * capacity;
	const void *allocation = malloc(sizeof(_CircularBuffer) + size_of_data);
	CircularBuffer cb = (CircularBuffer)allocation;

	cb->read_offset = 0;
	cb->write_offset = 0;
	cb->item_size = item_size;
	cb->item_count = 0;
	cb->is_circled = false;
	memset(&cb->delete_info, 0, sizeof(ElementDeleteInfo));
	cb->end_marker = cb->data + size_of_data;

	return cb;
}

void CircularBuffer_SetDeleter
(
	CircularBuffer cb,
	CircularBuffer_ElementFree deleter,
	void *user_data
) {
	ASSERT(cb);
	cb->delete_info.deleter = deleter;
	cb->delete_info.user_data = user_data;
}

uint64_t CircularBuffer_GetCapacity(const CircularBuffer cb) {
	ASSERT(cb);
	if (!cb) {
		return 0;
	}

	return (cb->end_marker - cb->data) / cb->item_size;
}

bool CircularBuffer_SetCapacity
(
	CircularBuffer *cb_ptr,
	const uint64_t new_capacity
) {
	ASSERT(cb_ptr && *cb_ptr);
	ASSERT(new_capacity);
	if (!cb_ptr || !*cb_ptr || !new_capacity) {
		return false;
	}
	CircularBuffer cb = *cb_ptr;
	if (new_capacity == CircularBuffer_GetCapacity(cb)) {
		// NOOP, the original buffer already has the same capacity.
		return true;
	}

	CircularBuffer new_cb = CircularBuffer_New(cb->item_size, new_capacity);
	ASSERT(new_cb);
	if (!new_cb) {
		return false;
	}

	/*
	The Reallocation one-to-one would be simple:
	We should copy from 6 till 9, then from 0 till 6.
	 __________
	|0123456789|
	 ^^^^^^^^^^
       W   R
	   2   6

	However, here we are also taking into account the new capacity. Suppose it
	is 5 now, instead of 10. We should allocate a new circular buffer with the
	size of 5 and copy the last (from the end of the previous circular buffer)
	5 unread elements: 7, 8, 9, 0, 1, leaving the first unread one (the sixth)
	out, as we only care about the most recent elements in the circular
	buffer, not really the old ones.

	We should copy those to the beginning of the new circular buffer as it
	doesn't make sense to do it in any other way. Hence, the first unread
	element will be at the 0 index and the write position will be the number of
	elements we copied. In this case it is five, so with a circle we set it to
	0 as well. But this isn't always the case: if in the original circular
	buffer there were less elements than the capacity, for example, just three
	out of 10 and we reallocated with the limit of five, then the write offset
	should point to the third (3) index, as 3 and 4 indices will be empty.
	*/

    /*
	This is the byte offset in the old circular buffer, where we should
	start copying "capacity" elements from, if there are so many until the
	end of the contiguous array.
	This should be the "new capacity" elements from the end of the old circular
	buffer, so we calculate those as:

		copy_from = "old write offset" - new_capacity

	If it goes below the zero, we should split the copy phase into two stages:
		1. Copy from zero till the old write offset.
		2. Copy from the end of the previous contiguous array - "number of
		   elements left to copy".
	This should be done in the reverse order, obviously, so first the 2. and
	1. after.
	*/
    // Here we can't attempt to copy more elements than we actually have, so
	// we need to know how many we can.
	const uint64_t number_of_elements = MIN(new_capacity, cb->item_count);
	uint64_t last_written_offset = cb->write_offset;
	// // The element under this index was written to the last:
	// uint64_t last_written_offset = cb->write_offset - 1;
	// if (last_written_offset > cb->item_count) {
	// 	last_written_offset = cb->item_count;
	// }
	// Check if we have enough elements to copy with just one attempt.
	// If we can, this is simple:
	if (last_written_offset >= number_of_elements) {
		// Just copy the last "number_of_elements" elements.
		const uint64_t copy_from
			= cb->data + (cb->write_offset - number_of_elements) * cb->item_size;
		const uint64_t copy_amount
			= number_of_elements * cb->item_size;
		memcpy(new_cb->data, copy_from, copy_amount);
	} else {
		// Else if we can't, we split into the two stages as explained above.
		// This is going to be the number of elements we should copy from the
		// beginning of the old buffer.
		const uint64_t how_many_from_the_write_offset
			= cb->write_offset;
		const uint64_t how_many_from_the_end_of_buffer
			= number_of_elements - how_many_from_the_write_offset;
		// 2. Copy from the end of the buffer, "the remainder":
		const uint64_t first_stage_bytes
			= how_many_from_the_end_of_buffer * cb->item_size;
		const uint64_t first_stage_from
			= cb->end_marker - first_stage_bytes;
		memcpy(new_cb->data, first_stage_from, first_stage_bytes);
		// 1. Copy from the zeroth offset till the write offset.
		const uint64_t second_stage_bytes
			= how_many_from_the_write_offset * cb->item_size;
		const uint64_t second_stage_from
			= cb->data;
		void *second_stage_to = new_cb->data + first_stage_bytes;
		memcpy(second_stage_to, second_stage_from, second_stage_bytes);
	}

	// The very first unread element is copied directly to the beginning.
	new_cb->read_offset = 0;
	// The write offset is the last pushed element to the new buffer.
	new_cb->write_offset = number_of_elements % new_capacity;
	new_cb->item_count = number_of_elements;
	new_cb->is_circled = new_cb->write_offset == 0;
	memcpy(&new_cb->delete_info, &cb->delete_info, sizeof(ElementDeleteInfo));

	CircularBuffer_Free(cb);
	*cb_ptr = new_cb;

	return true;
}

// Increments the offset with a circle back.
// Returns true if the value has circled, false otherwise.
static bool _IncrementOffsetCircular
(
	uint64_t *offset_ptr,
	const uint64_t capacity
) {
	ASSERT(offset_ptr);
	if (!offset_ptr) {
		return false;
	}

	*offset_ptr = *offset_ptr + 1;
	if (unlikely(*offset_ptr >= capacity)) {
		*offset_ptr = 0;
		return true;
	}

	return false;
}

void CircularBuffer_Add
(
	CircularBuffer cb,
	const void *item
) {
	ASSERT(cb);
	if (!cb) {
		return;
	}

    void *destination = _CircularBuffer_ItemAtWriteOffset(cb);

	// If we are overwriting, use the deleter.
	if (cb->item_count > cb->write_offset) {
		_DeleteElement(destination, cb->delete_info);
	}
	memcpy(destination, item, cb->item_size);

	const uint64_t capacity = CircularBuffer_GetCapacity(cb);

	if (cb->is_circled && unlikely(cb->write_offset == cb->read_offset)) {
		_IncrementOffsetCircular(&cb->read_offset, capacity);
	}

	cb->item_count = MIN(cb->item_count + 1, capacity);

	const bool writes_circled = _IncrementOffsetCircular(&cb->write_offset, capacity);

	if (writes_circled) {
		cb->is_circled = true;
	}
}

void CircularBuffer_Read
(
	CircularBuffer cb,  // buffer to remove item from
	void *item          // [output] pointer populated with removed item
) {
	const uint64_t capacity = CircularBuffer_GetCapacity(cb);

	if (unlikely(cb->read_offset == cb->write_offset)) {
		if (cb->is_circled) {
			cb->is_circled = false;
		} else {
			return;
		}
	}

	const void* source = _CircularBuffer_ItemAtReadOffset(cb);
	memcpy(item, source, cb->item_size);

	const bool reads_circled = _IncrementOffsetCircular(&cb->read_offset, capacity);
	if (cb->is_circled && reads_circled) {
		cb->is_circled = false;
	}
}

void CircularBuffer_ViewAll
(
	CircularBuffer buffer,
	CircularBuffer_ViewAllCallback callback,
	void *user_data
) {
	ASSERT(buffer);
	ASSERT(callback);
	if (!buffer || !callback) {
		return;
	}

	uint64_t read_view_offset = buffer->read_offset;
	bool is_circled = buffer->is_circled;
	while (true) {
		if (read_view_offset == buffer->write_offset) {
			if (is_circled) {
				is_circled = false;
			} else {
				break;
			}
		}
		void *item = _CircularBuffer_ItemAtOffset(buffer, read_view_offset);
		callback(user_data, item);
		if (buffer->data + ++read_view_offset * buffer->item_size >= buffer->end_marker) {
			read_view_offset = 0;
		}
	}
	buffer->read_offset = read_view_offset;
	buffer->is_circled = false;
}

void *CircularBuffer_ViewCurrent
(
	const CircularBuffer cb
) {
	ASSERT(cb);
	if (!cb) {
		return NULL;
	}
	if (CircularBuffer_ItemCount(cb) == 0) {
		return NULL;
	}
	return _CircularBuffer_ItemAtReadOffset(cb);
}

uint64_t CircularBuffer_ItemCount
(
	const CircularBuffer cb
) {
	ASSERT(cb);
	if (!cb) {
		return 0;
	}

	return cb->item_count;
}

uint64_t CircularBuffer_ItemSize
(
	const CircularBuffer cb
) {
	ASSERT(cb);
	if (!cb) {
		return 0;
	}
	return cb->item_size;
}

bool CircularBuffer_IsEmpty
(
	const CircularBuffer cb
) {
	ASSERT(cb);
	if (!cb) {
		return false;
	}
	return cb->item_count == 0;
}

static void _CircularBuffer_DeleteAllElements(CircularBuffer cb) {
	ASSERT(cb);
	if (!cb || !cb->delete_info.deleter) {
		return;
	}

	uint64_t read_offset = 0;
	while (read_offset < cb->item_count) {
		void *item = _CircularBuffer_ItemAtOffset(cb, read_offset);
		_DeleteElement(item, cb->delete_info);
		++read_offset;
	}
}

void CircularBuffer_Free
(
	CircularBuffer cb
) {
	ASSERT(cb);
	if (cb) {
		_CircularBuffer_DeleteAllElements(cb);
		free(cb);
	}
}
