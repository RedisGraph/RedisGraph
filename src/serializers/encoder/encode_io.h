/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../util/pipe.h"
#include "../../redismodule.h"

typedef enum {
	IOEncoderType_RDB,
	IOEncoderType_Pipe
} IOEncoderType;

typedef struct IOEncoder {
	void *io;
	IOEncoderType t;
	void(*SaveFloat)(const struct IOEncoder *io, float value);
	void(*SaveDouble)(const struct IOEncoder *io, double value);
	void(*SaveSigned)(const struct IOEncoder *io, int64_t value);
	void(*SaveUnsigned)(const struct IOEncoder *io, uint64_t value);
	void(*SaveLongDouble)(const struct IOEncoder *io, long double value);
	void(*SaveStringBuffer)(const struct IOEncoder *io, const char *value, size_t len);
} IOEncoder;

IOEncoder *IOEncoder_New
(
	IOEncoderType t,
	void *io
);

void IOEncoder_SaveUnsigned
(
	const IOEncoder *io,
	uint64_t value
);

void IOEncoder_SaveSigned
(
	const IOEncoder *io,
	int64_t value
);

void IOEncoder_SaveDouble
(
	const IOEncoder *io,
	double value
);

void IOEncoder_SaveFloat
(
	const IOEncoder *io,
	float value
);

void IOEncoder_SaveLongDouble
(
	const IOEncoder *io,
	long double v
);

void IOEncoder_SaveStringBuffer
(
	const IOEncoder *io,
	const char *str,
	size_t len
);

void IOEncoder_Free
(
	IOEncoder *encoder
);
