/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include <stddef.h>

//typedef int Pipe[2];
typedef struct Pipe { int fd[2]; } Pipe;

Pipe *Pipe_Create(void);

//------------------------------------------------------------------------------
// Pipe WRITE
//------------------------------------------------------------------------------

void Pipe_WriteUnsigned
(
	Pipe *p,
	uint64_t value
);

void Pipe_WriteSigned
(
	Pipe *p,
	int64_t value
);

//void Pipe_WriteString
//(
//	Pipe p,
//	RedisModuleString *s
//);

void Pipe_WriteStringBuffer
(
	Pipe *p,
	const char *str,
	size_t len
);

void Pipe_WriteDouble
(
	Pipe *p,
	double value
);

void Pipe_WriteFloat
(
	Pipe *p,
	float value
);

void Pipe_WriteLongDouble
(
	Pipe *p,
	long double value
);

//------------------------------------------------------------------------------
// Pipe READ
//------------------------------------------------------------------------------

uint64_t Pipe_ReadUnsigned
(
	Pipe *p
);

int64_t Pipe_ReadSigned
(
	Pipe *p
);

char *Pipe_ReadStringBuffer
(
	Pipe *p,
	size_t *lenptr
);

//RedisModuleString *Pipe_ReadString
//(
//	Pipe p
//);

double Pipe_ReadDouble
(
	Pipe *p
);

float Pipe_ReadFloat
(
	Pipe *p
);

long double Pipe_ReadLongDouble
(
	Pipe *p
);

void Pipe_Free
(
	Pipe *p
);

