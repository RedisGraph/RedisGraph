/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../util/pipe.h"
#include "../../redismodule.h"
#include <stdbool.h>

typedef enum {
	IODecoderType_RDB,
	IODecoderType_Pipe
} IODecoderType;

typedef struct IODecoder {
	void *io;
	bool IOError;
	IODecoderType t;
	float(*LoadFloat)(struct IODecoder *io);
	double(*LoadDouble)(struct IODecoder *io);
	int64_t(*LoadSigned)(struct IODecoder *io);
	uint64_t(*LoadUnsigned)(struct IODecoder *io);
	long double(*LoadLongDouble)(struct IODecoder *io);
	char*(*LoadStringBuffer)(struct IODecoder *io, uint64_t *len);
} IODecoder;

IODecoder *IODecoder_New
(
	IODecoderType t,
	void *io
);

uint64_t IODecoder_LoadUnsigned
(
	IODecoder *io
);

int64_t IODecoder_LoadSigned
(
	IODecoder *io
);

double IODecoder_LoadDouble
(
	IODecoder *io
);

float IODecoder_LoadFloat
(
	IODecoder *io
);

long double IODecoder_LoadLongDouble
(
	IODecoder *io
);

char *IODecoder_LoadStringBuffer
(
	IODecoder *io,
	uint64_t *len
);

void IODecoder_Free
(
	IODecoder *encoder
);

