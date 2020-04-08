/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_without_server_events.h"
#include "v6/encode_v6.h"

void RdbSaveGraphContext_WithoutServerEvents(RedisModuleIO *rdb, void *value) {
	return RdbSaveGraphContext_v6(rdb, value);
}
