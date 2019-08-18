/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __GRAPH_RESULTSET_H__
#define __GRAPH_RESULTSET_H__

#include "resultset_statistics.h"
#include "../redismodule.h"
#include "../execution_plan/record.h"
#include "rax.h"
#include "./formatters/resultset_formatters.h"

#define RESULTSET_UNLIMITED 0
#define RESULTSET_OK 1
#define RESULTSET_FULL 0

typedef struct {
	RedisModuleCtx *ctx;
	GraphContext *gc;           /* Context used for mapping attribute strings and IDs */
	uint column_count;          /* Number of columns in result set. */
	bool compact;               /* Whether records should be returned in compact form. */
	bool header_emitted;        /* Whether a header row has been issued to the user. */
	const char **columns;       /* Field names for each column of results. */
	size_t recordCount;         /* Number of records introduced. */
	ResultSetStatistics stats;  /* ResultSet statistics. */
	ResultSetFormatter *formatter;  /* ResultSet data formatter. */
} ResultSet;

ResultSet *NewResultSet(RedisModuleCtx *ctx, bool compact);

void ResultSet_BuildColumns(ResultSet *set, AR_ExpNode **projections);

int ResultSet_AddRecord(ResultSet *set, Record r);

void ResultSet_ReportError(ResultSet *set, char *error);

void ResultSet_Replay(ResultSet *set);

void ResultSet_Free(ResultSet *set);

#endif
