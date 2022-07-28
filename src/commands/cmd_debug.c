/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
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

static void Debug_AUX(RedisModuleString **argv, int argc) {
	if(argc < 2) return;

	const char *arg = RedisModule_StringPtrLen(argv[1], NULL);

	if(strcmp(arg, "START") == 0) {
		ModuleEventHandler_AUXBeforeKeyspaceEvent();
	} else if(strcmp(arg, "END") == 0) {
		ModuleEventHandler_AUXAfterKeyspaceEvent();
	}
}

int Graph_Debug(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	ASSERT(ctx != NULL);
	ASSERT(graphs_in_keyspace != NULL);
	RedisModule_ReplicateVerbatim(ctx);

	if(strcmp(RedisModule_StringPtrLen(argv[1], NULL), "AUX") == 0) {
		Debug_AUX(argv + 1, argc - 1);
	}

	RedisModule_ReplyWithLongLong(ctx, aux_field_counter);

	return REDISMODULE_OK;
}

