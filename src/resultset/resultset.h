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
	RedisModuleCtx *ctx;            /* Redis context. */
	GraphContext *gc;               /* Context used for mapping attribute strings and IDs */
	uint column_count;              /* Number of columns in result set. */
	const char **columns;           /* Field names for each column of results. */
	uint *columns_record_map;       /* Mapping between column name and record index.*/
	DataBlock *cells;               /* Accumulated cells */
	double timer[2];                /* Query runtime tracker. */
	ResultSetStatistics stats;      /* ResultSet statistics. */
	ResultSetFormatterType format;  /* Result-set format; compact/verbose/nop. */
	ResultSetFormatter *formatter;  /* ResultSet data formatter. */
} ResultSet;

// map each column to a record index
// such that when resolving resultset row i column j we'll extract
// data from record at position columns_record_map[j]
void ResultSet_MapProjection
(
	ResultSet *set,  // resultset to init mappings for
	const Record r   // record to map
);

// create a new result set
ResultSet *NewResultSet
(
	RedisModuleCtx *ctx,
	ResultSetFormatterType format  // resultset format
);

// returns number of rows in result-set
uint64_t ResultSet_RowCount
(
	const ResultSet *set  // resultset to inquery
);

// add a new row to resultset
int ResultSet_AddRecord
(
	ResultSet *set,  // resultset to extend
	Record r         // record containing projected data
);

// update resultset index creation statistics
void ResultSet_IndexCreated
(
	ResultSet *set,  // resultset to update
	int status_code  // index creation status code
);

// update resultset index deleted statistics
void ResultSet_IndexDeleted
(
	ResultSet *set,  // resultset to update
	int status_code  // index deletion status code
);

// update resultset cache execution statistics
void ResultSet_CachedExecution
(
	ResultSet *set  // resultset to update
);

// flush resultset to network
void ResultSet_Reply
(
	ResultSet *set  // resultset to reply with
);

// free resultset
void ResultSet_Free
(
	ResultSet *set  // resultset to free
);

