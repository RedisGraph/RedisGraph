/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once


// a binary buffer acts like a contiguous block of memory
// although internally it might be splitted into several sub buffers
// the buffer expose a set of APIs for populating the buffer with data
// the data is written as is without any modifications in a lieanr fashion

typedef struct _BinaryBuffer* BinaryBuffer;

// buffer minimum capacity
#define BUFFER_DATA_SIZE 1<<14 // 16K

// create a new BinaryBuffer
BinaryBuffer BinaryBuffer_New(void);

//------------------------------------------------------------------------------
// populate API
//------------------------------------------------------------------------------

// write bytes to buffer
void BinaryBuffer_WriteBytes
(
	BinaryBuffer buf,           // buffer to populate
	const unsigned char *data,  // data to write
	size_t len                  // number of bytes to write
);

// write byte to buffer
void BinaryBuffer_WriteByte
(
	BinaryBuffer buf,   // buffer to populate
	char b              // byte to write
);

// write integer to buffer
void BinaryBuffer_WriteInt
(
	BinaryBuffer buf,   // buffer to populate
	int64_t n           // integer to write
);

// write unsigned integer to buffer
void BinaryBuffer_WriteUnsignedInt
(
	BinaryBuffer buf,   // buffer to populate
	uint64_t n          // unsigned integer to write
);

// write dobule to buffer
void BinaryBuffer_WriteDouble
(
	BinaryBuffer buf,   // buffer to populate
	double n            // double to write
);

// write string to buffer
void BinaryBuffer_WriteString
(
	BinaryBuffer buf,   // buffer to populate
	const char *s       // string to write
);

// assert unknown type
void BinaryBuffer_WriteAssert
(
	BinaryBuffer buf,   // buffer to populate
	...
);

#define BinaryBuffer_Write(buf, x)                                   \
	_Generic                                                         \
	(                                                                \
		(x),                                                         \
			size_t        : BinaryBuffer_WriteUnsignedInt ,          \
			int64_t       : BinaryBuffer_WriteInt      ,             \
			uint64_t      : BinaryBuffer_WriteUnsignedInt ,          \
			double        : BinaryBuffer_WriteDouble   ,             \
			char          : BinaryBuffer_WriteByte     ,             \
			unsigned char : BinaryBuffer_WriteByte     ,             \
			const char*   : BinaryBuffer_WriteString   ,             \
			default       : BinaryBuffer_WriteAssert                 \
	)                                                                \
	(buf, x)

//------------------------------------------------------------------------------
// Debug API
//------------------------------------------------------------------------------

// returns number of internal buffers
uint64_t BinaryBuffer_InternalBuffersCount
(
	const BinaryBuffer buff
);

// returns internal buffers
void BinaryBuffer_InternalBuffers
(
	const BinaryBuffer buff,  // buffer to query
	uint64_t n,               // length of each input array
	char **buffers,           // returned buffers
	size_t *counts,           // number of bytes written in each buffer
	size_t *caps              // capacity of each buffer
);

// free buffer
void BinaryBuffer_Free
(
	BinaryBuffer *buf  // buffer to free
);

