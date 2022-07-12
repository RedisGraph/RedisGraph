/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include "../redismodule.h"

#define STAT_NOT_SET -1

typedef struct {
	int labels_added;           // number of labels added as part of a create query
	int nodes_created;          // number of nodes created as part of a create query
	int properties_set;         // number of properties created as part of a create query
	int relationships_created;  // number of edges created as part of a create query
	int nodes_deleted;          // number of nodes removed as part of a delete query
	int relationships_deleted;  // number of edges removed as part of a delete query
	int indices_created;        // number of indices created
	int indices_deleted;        // number of indices deleted
	bool cached;                // indication for a cached query execution
} ResultSetStatistics;

// Checks to see if resultset-statistics indicate that a modification was made
bool ResultSetStat_IndicateModification
(
	const ResultSetStatistics *stats // resultset statistics to inquery
);

// initialize resultset statistics
void ResultSetStat_init
(
	ResultSetStatistics *stats  // resultset statistics to initialize
);

// emit resultset statistics
void ResultSetStat_emit
(
	RedisModuleCtx *ctx,              // redis module context
	const ResultSetStatistics *stats  // statistics to emit
);

// Clear result-set statistics
void ResultSetStat_Clear
(
	ResultSetStatistics *stats
);
