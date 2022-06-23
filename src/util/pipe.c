/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "pipe.h"
#include "rmalloc.h"
#include <unistd.h>

// pipe read end
#define PIPE_READ_END(p) ((p)->fd)[0]

// pipe write end
#define PIPE_WRITE_END(p) ((p)->fd)[1]

Pipe *Pipe_Create(void) {
	Pipe *p = rm_malloc(sizeof(Pipe));
	int res = pipe((int*)p);

	ASSERT(res == 0);

	return p;
}

// close write end of pipe
void Pipe_CloseWriteEnd
(
	Pipe *p
) {
	ASSERT(p != NULL);
	close(PIPE_WRITE_END(p));
}

// close read end of pipe
void Pipe_CloseReadEnd
(
	Pipe *p
) {
	ASSERT(p != NULL);
	close(PIPE_READ_END(p));
}

//------------------------------------------------------------------------------
// Pipe WRITE
//------------------------------------------------------------------------------

static ssize_t Pipe_Write
(
	Pipe *p,
	const void *buf,
	size_t count
) {
	ASSERT(buf != NULL);
	ASSERT(count > 0);

	return write(PIPE_WRITE_END(p), buf, count);
}

// macro for Pipe_Write* functions
#define PIPE_WRITE(name, type)                                              \
ssize_t Pipe_Write##name                                                    \
(                                                                           \
	Pipe *p,                                                                \
	type value                                                              \
) {                                                                         \
	return Pipe_Write(p, &value, sizeof(type));                             \
}

PIPE_WRITE (Unsigned  , uint64_t   )
PIPE_WRITE (Double    , double     )
PIPE_WRITE (LongDouble, long double)
PIPE_WRITE (Float     , float      )
PIPE_WRITE (Signed    , int64_t    )

ssize_t Pipe_WriteStringBuffer
(
	Pipe *p,
	const char *str,
	uint64_t len
) {
	// write both string length and string to pipe
	ssize_t res = Pipe_WriteUnsigned(p, len);
	if(res != -1) {
		res = Pipe_Write(p, str, sizeof(char)*len);
	}
	return res;
}

//------------------------------------------------------------------------------
// Pipe READ
//------------------------------------------------------------------------------

size_t Pipe_Read
(
	Pipe *p,
	void *buf,
	size_t count
) {
	ASSERT(buf != NULL);
	ASSERT(count > 0);

	ssize_t n = read(PIPE_READ_END(p), buf, count);
	ASSERT(n == count);

	return n;
}

// macro for Pipe_Read* functions
#define PIPE_READ(name, type)                                               \
ssize_t Pipe_Read##name                                                     \
(                                                                           \
	Pipe *p,                                                                \
	type *value                                                             \
) {                                                                         \
	return Pipe_Read(p, value, sizeof(type));                               \
}

PIPE_READ (Float     , float      )
PIPE_READ (Double    , double     )
PIPE_READ (Signed    , int64_t    )
PIPE_READ (Unsigned  , uint64_t   )
PIPE_READ (LongDouble, long double)

ssize_t Pipe_ReadStringBuffer
(
	Pipe *p,
	uint64_t *lenptr,
	char **value
) {
	// read string length from pipe
	uint64_t len;
	ssize_t res = Pipe_ReadUnsigned(p, &len);

	// failed to read from pipe
	if(res == 0) {
		*value = NULL;
		return res;
	}

	// set lenptr if caller requested it
	if(lenptr != NULL) {
		*lenptr = len;
	}

	// allocate buffer for string
	*value = rm_calloc(1, len);
	res = Pipe_Read(p, *value, len);

	// failed to read from pipe
	if(res == 0) {
		rm_free(*value);
		*value = NULL;
	}

	return res;
}

void Pipe_Free
(
	Pipe *p
) {
	close(PIPE_READ_END(p));
	close(PIPE_WRITE_END(p));

	rm_free(p);
}

