/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "rmalloc.h"
#include "binary_buffer.h"

// a binary buffer is a linked list of sub buffers
struct _BinaryBuffer {
	size_t       cap;    // data capacity
	size_t       count;  // number of bytes wrriten to buffer
	BinaryBuffer curr;   // current buffer to write to
	BinaryBuffer head;   // beginning of buffer chain
	BinaryBuffer prev;   // previous buffer
	BinaryBuffer next;   // following buffer
	char data[];         // buffer's data block
};

// creates a new BinaryBuffer as part of a buffer chain
static BinaryBuffer _BinaryBuffer_New
(
	BinaryBuffer parent,  // parent of new buffer
	size_t cap            // capacity of new buffer
) {
	BinaryBuffer buf = rm_malloc(sizeof(struct _BinaryBuffer) + cap);
	
	buf->cap   = cap;
	buf->head  = NULL;
	buf->curr  = NULL;
	buf->next  = NULL;
	buf->prev  = parent;
	buf->count = 0;

	// set parent's next pointer to newly created buffer
	// set head's current to new buffer
	if(parent != NULL) {
		buf->head        =  parent->head;
		parent->next     =  buf;
		buf->head->curr  =  buf;
	}

	return buf;
}

// create a new BinaryBuffer
BinaryBuffer BinaryBuffer_New(void) {

	BinaryBuffer buf = _BinaryBuffer_New(NULL, BUFFER_DATA_SIZE);

	// update head to self
	buf->head = buf;
	buf->curr = buf;

	return buf;
}

//------------------------------------------------------------------------------
// populate API
//------------------------------------------------------------------------------

// BinaryBuffer_WriteBytes is a "sink" method which all BinaryBuffer_Write*
// functions call e.g.
// BinaryBuffer_WriteString invokes BinaryBuffer_WriteBytes(buf, s, strlen(s));
//
// write 'len' bytes from 'data' into buffer
void BinaryBuffer_WriteBytes
(
	BinaryBuffer buf,           // buffer to populate
	const unsigned char *data,  // data to write
	size_t len                  // number of bytes to write
) {
	// validations
	ASSERT(len   >  0);
	ASSERT(buf   != NULL);
	ASSERT(data  != NULL);

	// TODO: a bit of a shame
	buf = buf->head->curr;

	// available space in current buffer
	size_t available = buf->cap - buf->count;

	// write at most 'available' bytes
	size_t n = (len > available) ? available : len;

	// copy data into buffer
	memcpy(buf->data + buf->count, data, n);

	// update number of bytes written to buffer
	buf->count += n;

	// see if buffer ran out of space and we've got more data to write
	len -= n;
	if(len > 0) {
		// buffer is full!
		// create a new buffer and write remaining data
		// new minimum capacity doubles previous buffer capacity
		size_t min_cap = buf->cap * 2;
		size_t cap = (len > min_cap) ? len : min_cap;
		buf = _BinaryBuffer_New(buf, cap);

		// copy remaining data into buffer
		memcpy(buf->data, data, len);

		// update number of bytes written to buffer
		buf->count += len;
	}
}

#define BinaryBuffer_WriteType(name, type)                            \
void BinaryBuffer_Write##name(BinaryBuffer buf, type x) {             \
	/*printf("in BinaryBuffer_Write_"#name"\n");*/                    \
	BinaryBuffer_WriteBytes(buf, (unsigned char*)&x, sizeof(type));   \
}

BinaryBuffer_WriteType(Byte, char)
BinaryBuffer_WriteType(Int, int64_t)
BinaryBuffer_WriteType(Double, double)
BinaryBuffer_WriteType(UnsignedInt, uint64_t)

// write string to buffer
void BinaryBuffer_WriteString
(
	BinaryBuffer buf,   // buffer to populate
	const char *s       // string to write
) {
	// TODO: should we includes '\0' byte?
	BinaryBuffer_WriteBytes(buf, (const unsigned char*)s, strlen(s));
}

// assert unknown type
void BinaryBuffer_WriteAssert
(
	BinaryBuffer buf,   // buffer to populate
	...
) {
	ASSERT("Unknown type!" && false);
}

//------------------------------------------------------------------------------
// Debug API
//------------------------------------------------------------------------------

// returns number of internal buffers
uint64_t BinaryBuffer_InternalBuffersCount
(
	const BinaryBuffer buff
) {
	ASSERT(buff != NULL);

	// _buff starts at the head of the buffer list
	BinaryBuffer _buff = buff->head;

	// acount each internal buffer
	uint64_t n = 0;

	do {
		n++;
		_buff = _buff->next;
	} while(_buff != NULL);

	return n;
}

// returns internal buffers
void BinaryBuffer_InternalBuffers
(
	const BinaryBuffer buff,  // buffer to query
	uint64_t n,               // length of each input array
	char **buffers,           // returned buffers
	size_t *counts,           // number of bytes written in each buffer
	size_t *caps              // capacity of each buffer
) {
	ASSERT(buff != NULL);
	ASSERT(n == BinaryBuffer_InternalBuffersCount(buff));

	// _buff starts at the head of the buffer list
	BinaryBuffer _buff = buff->head;

	for(uint64_t i = 0; i < n; i++) {
		caps[i]     =  _buff->cap;
		counts[i]   =  _buff->count;
		buffers[i]  =  _buff->data;

		_buff = _buff->next;
	}
}

// free buffer
void BinaryBuffer_Free
(
	BinaryBuffer *buf  // buffer to free
) {
	ASSERT(buf  != NULL);
	ASSERT(*buf != NULL);

	BinaryBuffer cur = (*buf)->head;

	while(cur != NULL) {
		BinaryBuffer next = cur->next;
		rm_free(cur);
		cur = next;
	}

	// nullify freed buffer
	*buf = NULL;
}

