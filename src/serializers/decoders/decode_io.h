/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../util/pipe.h"
#include "../../redismodule.h"

typedef enum {
	IODecoderType_RDB,
	IODecoderType_Pipe
} IODecoderType;

typedef struct IODecoder {
	void *io;
	IODecoderType t;
	float(*LoadFloat)(const struct IODecoder *io);
	double(*LoadDouble)(const struct IODecoder *io);
	int64_t(*LoadSigned)(const struct IODecoder *io);
	uint64_t(*LoadUnsigned)(const struct IODecoder *io);
	long double(*LoadLongDouble)(const struct IODecoder *io);
	char*(*LoadStringBuffer)(const struct IODecoder *io, size_t *len);
} IODecoder;

IODecoder *IODecoder_New
(
	IODecoderType t,
	void *io
);

uint64_t IODecoder_LoadUnsigned
(
	const IODecoder *io
);

int64_t IODecoder_LoadSigned
(
	const IODecoder *io
);

double IODecoder_LoadDouble
(
	const IODecoder *io
);

float IODecoder_LoadFloat
(
	const IODecoder *io
);

long double IODecoder_LoadLongDouble
(
	const IODecoder *io
);

char *IODecoder_LoadStringBuffer
(
	const IODecoder *io,
	size_t *len
);

void IODecoder_Free
(
	IODecoder *encoder
);

