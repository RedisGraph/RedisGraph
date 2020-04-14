/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_with_server_events.h"
#include "v7/decode_v7.h"

GraphContext *RdbLoadGraphContext_WithServerEvents(RedisModuleIO *rdb) {
	return RdbLoadGraphContext_v7(rdb);
}
