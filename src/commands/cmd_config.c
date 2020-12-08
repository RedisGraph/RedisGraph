/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cmd_config.h"
#include "config.h"
#include <string.h>

void MGraph_Config_get(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// return the given config's value to the user
	Config_Option_Field config_field;
	const char *config_name = RedisModule_StringPtrLen(argv[2], NULL);
	if(!Config_Contains_field(config_name, &config_field)) {
		RedisModule_ReplyWithError(ctx, "Unknown configuration field");
		return;
	}

	long long value = 0;

	if(Config_Option_get(config_field, &value)) {
		RedisModule_ReplyWithArray(ctx, 2);
		RedisModule_ReplyWithCString(ctx, config_name);
		RedisModule_ReplyWithLongLong(ctx, value);
	} else {
		RedisModule_ReplyWithError(ctx, "Configuration field was not found");
	}
}

void MGraph_Config_set(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// list of configurable fields
	size_t n_whitelist = 1;
	Config_Option_Field whitelist[1] = { Config_RESULTSET_MAX_SIZE };

	//--------------------------------------------------------------------------
	// retrieve and validate config field
	//--------------------------------------------------------------------------

	Config_Option_Field config_field;
	const char *config_name = RedisModule_StringPtrLen(argv[2], NULL);

	if(!Config_Contains_field(config_name, &config_field)) {
		RedisModule_ReplyWithError(ctx, "Unknown configuration field");
		return;
	}

	// ensure field is whitelisted
	bool configurable_field = false;
	for(int i = 0; i < n_whitelist; i++) {
		if(whitelist[i] == config_field) {
			configurable_field = true;
			break;
		}
	}
	
	// field is not allowed to be reconfigured
	if(!configurable_field) {
		RedisModule_ReplyWithError(ctx, "Field can not be re-configured");
		return;
	}

	// set the value of given config
	RedisModuleString *value = argv[3];

	if(Config_Option_set(config_field, value)) {
		RedisModule_ReplyWithSimpleString(ctx, "OK");
	} else {
		RedisModule_ReplyWithError(ctx, "Failed to set config value");
	}
}

int MGraph_Config(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// CONFIG <GET|SET> <NAME> [value]
	if(argc < 3) return RedisModule_WrongArity(ctx);

	const char *action = RedisModule_StringPtrLen(argv[1], NULL);

	if(!strcasecmp(action, "GET")) {
		// CONFIG GET <NAME>
		if(argc != 3) return RedisModule_WrongArity(ctx);
		MGraph_Config_get(ctx, argv, argc);
	} else if(!strcasecmp(action, "SET")) {
		// CONFIG SET <NAME> [value]
		if(argc != 4) return RedisModule_WrongArity(ctx);
		MGraph_Config_set(ctx, argv, argc);
	} else {
		RedisModule_ReplyWithError(ctx, "Unknown subcommand for GRAPH.CONFIG");
	}

	return REDISMODULE_OK;
}

