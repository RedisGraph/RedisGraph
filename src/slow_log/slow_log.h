/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#define SLOW_LOG_SIZE 10

#include <pthread.h>

#include "../util/heap.h"
#include "../redismodule.h"
#include "../../deps/rax/rax.h"

// Slowlog item.
typedef struct {
    char *cmd;          // Redis command.
    time_t time;        // Item creation time.
	char *query;        // Query.
    char *graph_id;     // Graph ID.
	double latency;     // How much time query was processed.
} SlowLogItem;

// Slowlog, maintains N slowest queries.
typedef struct {
     uint count;                // Length of lookup, min_heap and locks arrays.
     rax **lookup;              // Array of item lookup table.
     heap_t **min_heap;         // Array of minimum heap of items.
     pthread_mutex_t *locks;    // Array of locks.
} SlowLog;

// Introduce item to slow log.
void SlowLog_Add(const char *cmd, const char *graph_id, const char *query, double latency);

// Replies with slow log content.
void SlowLog_Replay(RedisModuleCtx *ctx);
