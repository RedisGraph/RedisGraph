/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_previous.h"
#include "prev/decoders.h"

GraphContext *Decode_Previous(RedisModuleIO *rdb, int encver) {
	switch(encver) {
	case 4:
		return RdbLoadGraphContext_v4(rdb);
	case 5:
		return RdbLoadGraphContext_v5(rdb);
	case 6:
		return RdbLoadGraphContext_v6(rdb);
	case 7:
		return RdbLoadGraphContext_v7(rdb);
	default:
		ASSERT(false && "attempted to read unsupported RedisGraph version from RDB file.");
		return NULL;
	}
}

