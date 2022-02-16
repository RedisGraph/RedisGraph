/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "resultset_statistics.h"
#include "../redismodule.h"
#include "../execution_plan/record.h"
#include "rax.h"
#include "./formatters/resultset_formatters.h"

#define RESULTSET_OK 1
#define RESULTSET_FULL 0

typedef struct {
	RedisModuleCtx *ctx;            // redis context
	GraphContext *gc;               // context used for mapping attribute strings and IDs
	uint column_count;              // number of columns in result set
	const char **columns;           // field names for each column of results
	uint *columns_record_map;       // mapping between column name and record index
	DataBlock *cells;               // accumulated cells
	double timer[2];                // query runtime tracker
	ResultSetStatistics stats;      // resultSet statistics
	ResultSetFormatterType format;  // result-set format; compact/verbose/nop
	ResultSetFormatter *formatter;  // resultSet data formatter
} ResultSet;

// map each column to a record index
// such that when resolving resultset row i column j we'll extract
// data from record at position columns_record_map[j]
void ResultSet_MapProjection
(
	ResultSet *set,
	const Record r
);

// allocate new result set and initialize it
ResultSet *NewResultSet
(
	RedisModuleCtx *ctx,
	ResultSetFormatterType format
);

// returns number of rows in result-set
uint64_t ResultSet_RowCount
(
	const ResultSet *set
);

// add a record to result set
int ResultSet_AddRecord
(
	ResultSet *set,
	Record r
);

// increment the index created stats
void ResultSet_IndexCreated
(
	ResultSet *set,
	int status_code
);

// increment the index deleted stats
void ResultSet_IndexDeleted
(
	ResultSet *set,
	int status_code
);

// set the cache execution stats
void ResultSet_CachedExecution
(
	ResultSet *set
);

// return the result to the client
void ResultSet_Reply
(
	ResultSet *set
);

// report execution timing
void ResultSet_ReportQueryRuntime
(
	RedisModuleCtx *ctx
);

// clear result set stats
void ResultSet_Clear
(
	ResultSet *set
);

// free the result set
void ResultSet_Free
(
	ResultSet *set
);
