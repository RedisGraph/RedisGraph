/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_previous.h"
#include "../../../../version.h"

GraphContext *Decode_Previous(RedisModuleIO *rdb, int encver) {
	switch(encver) {
	case 4:
		return RdbLoadGraphContext_v4(rdb);
	case 5:
		return RdbLoadGraphContext_v5(rdb);
	default:
		assert(false);
	}

	return NULL;
}
