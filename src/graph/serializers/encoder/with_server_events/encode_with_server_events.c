/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_with_server_events.h"
#include "v7/encode_v7.h"

void RdbSaveGraphContext_WithServerEvents(RedisModuleIO *rdb, void *value) {
	return RdbSaveGraphContext_v7(rdb, value);
}
