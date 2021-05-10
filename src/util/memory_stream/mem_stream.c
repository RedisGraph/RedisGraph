/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "./mem_stream.h"

#define STREAM_READ_END(s) (s)->read_fd
#define STREAM_WRITE_END(s) (s)->write_fd

struct MemStream_opaque
{
	int read_fd;   // read end of stream
	int write_fd;  // write end of stream
};

// create a new read/write memory stream
MemStream Stream_New
(
	void
) {
	int pipefd[2];
	assert(pipe(pipefd) == 0);

	MemStream stream = malloc(sizeof(struct MemStream_opaque));
	assert(stream != NULL);

	STREAM_READ_END(stream) = pipefd[0];
	STREAM_WRITE_END(stream) = pipefd[1];

	return stream;
}

// creates a read only stream from input stream
MemStream Stream_ReadOnly
(
	const MemStream stream  // stream from which to create a new readonly stream
) {
	assert(stream != NULL);
	
	MemStream ro_stream = malloc(sizeof(struct MemStream_opaque));
	assert(ro_stream != NULL);

	STREAM_READ_END(ro_stream)   =  STREAM_READ_END(stream);
	STREAM_WRITE_END(ro_stream)  =  0;

	return ro_stream;
}

// creates a write only stream from input stream
MemStream Stream_WriteOnly
(
	const MemStream stream // stream from which to create a new writeonly stream
) {
	assert(stream != NULL);
	
	MemStream wo_stream = malloc(sizeof(struct MemStream_opaque));
	assert(wo_stream != NULL);

	STREAM_READ_END(wo_stream)   =  0;
	STREAM_WRITE_END(wo_stream)  =  stream->write_fd;

	return wo_stream;
}
  
//------------------------------------------------------------------------------
// Stream read
//------------------------------------------------------------------------------

// read unsigned integer from stream
uint64_t Stream_ReadUnsigned
(
	MemStream stream
) {
	assert(stream != NULL);

	uint64_t n;

	size_t nbytes = read(STREAM_READ_END(stream), &n, 8);
	assert(nbytes == 8);

	return n;
}

// read signed integer from stream
int64_t Stream_ReadSigned
(
	MemStream stream
) {
	assert(stream != NULL);

	int64_t n;

	size_t nbytes = read(STREAM_READ_END(stream), &n, 8);
	assert(nbytes == 8);

	return n;
}

// read double from stream
double Stream_ReadDouble
(
	MemStream stream
) {

	assert(stream != NULL);

	double d;

	size_t nbytes = read(STREAM_READ_END(stream), &d, 8);
	assert(nbytes == 8);

	return d;
}

// read string from stream
char *Stream_ReadString
(
	MemStream stream,
	size_t *len
) {
	assert(stream != NULL);

	uint64_t l = Stream_ReadUnsigned(stream);
	char *s = malloc(sizeof(char) * l);
	assert(s != NULL);

	size_t nbytes = read(STREAM_READ_END(stream), &s, l);
	assert(nbytes == l);

	if(len != NULL) *len = l;

	return s;
}

//------------------------------------------------------------------------------
// Stream write
//------------------------------------------------------------------------------

// write unsigned integer from stream
int Stream_WriteUnsigned
(
	MemStream stream,  // stream to write to
	uint64_t n         // unsigned integer to write
) {
	assert(stream != NULL);

	return write(STREAM_WRITE_END(stream), &n, 8) == 8;
}

// write signed integer from stream
int Stream_WriteSigned
(
	MemStream stream,  // stream to write to
	int64_t n
) {
	assert(stream != NULL);

	return write(STREAM_WRITE_END(stream), &n, 8) == 8;
}

// write double from stream
int Stream_WriteDouble
(
	MemStream stream,  // stream to write to
	double d
) {

	assert(stream != NULL);

	return write(STREAM_WRITE_END(stream), &d, 8) == 8;
}

// write string from stream
int Stream_WriteString
(
	MemStream stream,  // stream to write to
	const char *str,   // string to write
	size_t len         // string length
) {

	assert(stream != NULL);
	assert(str != NULL);

	return write(STREAM_WRITE_END(stream), &str, len) == len;
}

