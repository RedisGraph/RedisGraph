/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_previous.h"
#include "v4/decode_v4.h"
#include "v5/decode_v5.h"

GraphContext *Decode_Previous(RedisModuleIO *rdb, int encver) {
	switch(encver) {
	case 4:
		return RdbLoadGraphContext_v4(rdb);
	case 5:
		return RdbLoadGraphContext_v5(rdb);
	default:
		assert(false && "attempted to read unsupported RedisGraph version from RDB file.");
	}

	return NULL;
}
