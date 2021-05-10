/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

typedef struct MemStream_opaque *MemStream;

// create a new read/write memory stream
MemStream Stream_New
(
	void
);

// creates a read only stream from input stream
MemStream Stream_ReadOnly
(
	const MemStream stream // stream from which to create a new readonly stream
);

// creates a write only stream from input stream
MemStream Stream_WriteOnly
(
	const MemStream stream // stream from which to create a new writeonly stream
);

//------------------------------------------------------------------------------
// Stream read
//------------------------------------------------------------------------------

// read unsigned integer from stream
uint64_t Stream_ReadUnsigned
(
	MemStream stream
);

// read signed integer from stream
int64_t Stream_ReadSigned
(
	MemStream stream
);

// read double from stream
double Stream_ReadDouble
(
	MemStream stream
);

// read string from stream
char *Stream_ReadString
(
	MemStream stream,
	size_t *len
);


//------------------------------------------------------------------------------
// Stream write
//------------------------------------------------------------------------------

// write unsigned integer from stream
int Stream_WriteUnsigned
(
	MemStream stream,  // stream to write to
	uint64_t n         // unsigned integer to write
);

// write signed integer from stream
int Stream_WriteSigned
(
	MemStream stream,  // stream to write to
	int64_t n
);

// write double from stream
int Stream_WriteDouble
(
	MemStream stream,  // stream to write to
	double d
);

// write string from stream
int Stream_WriteString
(
	MemStream stream,  // stream to write to
	const char *str,   // string to write
	size_t len         // string length
);

