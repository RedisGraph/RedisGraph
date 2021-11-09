/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "../util/arr.h"
#include "../redismodule.h"
#include "../graph/graphcontext.h"
#include "../module_event_handlers.h"

void ModuleEventHandler_AUXBeforeKeyspaceEvent(void);
void ModuleEventHandler_AUXAfterKeyspaceEvent(void);

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;
extern uint aux_field_counter;

int Graph_Debug(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	ASSERT(ctx != NULL);
	// ASSERT(graphs_in_keyspace != NULL);
	RedisModule_ReplicateVerbatim(ctx);

	if(strcmp(RedisModule_StringPtrLen(argv[1], NULL), "AUX") == 0) {
		if(strcmp(RedisModule_StringPtrLen(argv[2], NULL), "START") == 0) {
			ModuleEventHandler_AUXBeforeKeyspaceEvent();
		} else if(strcmp(RedisModule_StringPtrLen(argv[2], NULL), "END") == 0) {
			ModuleEventHandler_AUXAfterKeyspaceEvent();
		}
	}

	RedisModule_ReplyWithLongLong(ctx, aux_field_counter);

	return REDISMODULE_OK;
}
