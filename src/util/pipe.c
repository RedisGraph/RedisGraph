/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "pipe.h"
#include "rmalloc.h"
#include <unistd.h>

// get read end of pipe
#define PIPE_READ_END(p) (p)[0]

// get write end of pipe
#define PIPE_WRITE_END(p) (p)[1]

Pipe *Pipe_Create(void)
{
	Pipe *p = rm_malloc(sizeof(Pipe));
	int res = pipe((int*)p);

	ASSERT(res == 0);

	return p;
}

//------------------------------------------------------------------------------
// Pipe WRITE
//------------------------------------------------------------------------------

static void Pipe_Write
(
	Pipe *p,
	const void *buf,
	size_t count
) {
	ASSERT(buf != NULL);
	ASSERT(count > 0);

	ssize_t n = write(PIPE_WRITE_END(p->fd), buf, count);
	ASSERT(n == count);
}

#define PIPE_WRITE(name, type)                                              \
void Pipe_Write##name                                                       \
(                                                                           \
	Pipe *p,                                                           \
	type value                                                              \
) {                                                                         \
	Pipe_Write(p, &value, sizeof(type));                                    \
}

PIPE_WRITE (Unsigned  , uint64_t   )
PIPE_WRITE (Double    , double     )
PIPE_WRITE (LongDouble, long double)
PIPE_WRITE (Float     , float      )
PIPE_WRITE (Signed    , int64_t    )

void Pipe_WriteString
(
	Pipe *p,
	RedisModuleString *s
) {
	ASSERT(false);
}

void Pipe_WriteStringBuffer
(
	Pipe *p,
	const char *str,
	size_t len
) {
	Pipe_Write(p, str, sizeof(char)*len);
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

	ssize_t n = read(PIPE_READ_END(p->fd), buf, count);
	ASSERT(n == count);

	return n;
}

#define PIPE_READ(name, type)                                               \
type Pipe_Read##name                                                        \
(                                                                           \
	Pipe *p                                                            \
) {                                                                         \
	type value;                                                             \
	Pipe_Read(p, &value, sizeof(type));                                     \
	return value;                                                           \
}

PIPE_READ (Float     , float      )
PIPE_READ (Double    , double     )
PIPE_READ (Signed    , int64_t    )
PIPE_READ (Unsigned  , uint64_t   )
PIPE_READ (LongDouble, long double)

char *Pipe_ReadStringBuffer
(
	Pipe *p,
	size_t *lenptr
) {
	ASSERT(lenptr != NULL);

	char *str = rm_calloc(1, *lenptr);
	*lenptr = Pipe_Read(p, str, *lenptr);

	return str;
}

RedisModuleString *Pipe_ReadString
(
	Pipe *p
) {
	ASSERT(false);
	return NULL;
}

void Pipe_Free
(
	Pipe *p
) {
	close(p->fd[0]);
	close(p->fd[1]);

	rm_free(p);
}

