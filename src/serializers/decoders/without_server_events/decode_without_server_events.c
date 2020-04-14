/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_without_server_events.h"
#include "v6/decode_v6.h"

GraphContext *RdbLoadGraphContext_WithoutServerEvents(RedisModuleIO *rdb) {
	return RdbLoadGraphContext_v6(rdb);
}
