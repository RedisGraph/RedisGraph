/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct Pipe { int fd[2]; } Pipe;

Pipe *Pipe_Create(void);

// close write end of pipe
void Pipe_CloseWriteEnd
(
	Pipe *p
);

// close read end of pipe
void Pipe_CloseReadEnd
(
	Pipe *p
);

//------------------------------------------------------------------------------
// Pipe WRITE
//------------------------------------------------------------------------------

ssize_t Pipe_WriteUnsigned
(
	Pipe *p,
	uint64_t value
);

ssize_t Pipe_WriteSigned
(
	Pipe *p,
	int64_t value
);

ssize_t Pipe_WriteStringBuffer
(
	Pipe *p,
	const char *str,
	uint64_t len
);

ssize_t Pipe_WriteDouble
(
	Pipe *p,
	double value
);

ssize_t Pipe_WriteFloat
(
	Pipe *p,
	float value
);

ssize_t Pipe_WriteLongDouble
(
	Pipe *p,
	long double value
);

//------------------------------------------------------------------------------
// Pipe READ
//------------------------------------------------------------------------------

ssize_t Pipe_ReadUnsigned
(
	Pipe *p,
	uint64_t *value
);

ssize_t Pipe_ReadSigned
(
	Pipe *p,
	int64_t *value
);

ssize_t Pipe_ReadStringBuffer
(
	Pipe *p,
	uint64_t *lenptr,
	char **value
);

ssize_t Pipe_ReadDouble
(
	Pipe *p,
	double *value
);

ssize_t Pipe_ReadFloat
(
	Pipe *p,
	float *value
);

ssize_t Pipe_ReadLongDouble
(
	Pipe *p,
	long double *value
);

void Pipe_Free
(
	Pipe *p
);

