/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../grouping/group_cache.h"
#include "../arithmetic/aggregate.h"

static void _ResultSet_ReplayStats(RedisModuleCtx *ctx, ResultSet *set) {
	char buff[512] = {0};
	size_t resultset_size = 1; /* query execution time. */
	int buflen;

	if(set->stats.labels_added > 0) resultset_size++;
	if(set->stats.nodes_created > 0) resultset_size++;
	if(set->stats.properties_set > 0) resultset_size++;
	if(set->stats.relationships_created > 0) resultset_size++;
	if(set->stats.nodes_deleted > 0) resultset_size++;
	if(set->stats.relationships_deleted > 0) resultset_size++;

	RedisModule_ReplyWithArray(ctx, resultset_size);

	if(set->stats.labels_added > 0) {
		buflen = sprintf(buff, "Labels added: %d", set->stats.labels_added);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.nodes_created > 0) {
		buflen = sprintf(buff, "Nodes created: %d", set->stats.nodes_created);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.properties_set > 0) {
		buflen = sprintf(buff, "Properties set: %d", set->stats.properties_set);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.relationships_created > 0) {
		buflen = sprintf(buff, "Relationships created: %d", set->stats.relationships_created);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.nodes_deleted > 0) {
		buflen = sprintf(buff, "Nodes deleted: %d", set->stats.nodes_deleted);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.relationships_deleted > 0) {
		buflen = sprintf(buff, "Relationships deleted: %d", set->stats.relationships_deleted);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	// Emit query execution time.
	ResultSet_ReportQueryRuntime(ctx);
}

static void _ResultSet_ReplyWithPreamble(ResultSet *set, const Record r) {
	assert(set->recordCount == 0);

	// Prepare a response containing a header, records, and statistics
	RedisModule_ReplyWithArray(set->ctx, 3);

	// Emit the table header using the appropriate formatter
	set->formatter->EmitHeader(set->ctx, set->columns, r);

	set->header_emitted = true;

	// We don't know at this point the number of records we're about to return.
	RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
}

ResultSet *NewResultSet(RedisModuleCtx *ctx, bool compact) {
	ResultSet *set = rm_malloc(sizeof(ResultSet));
	set->ctx = ctx;
	set->gc = QueryCtx_GetGraphCtx();
	set->compact = compact;
	set->formatter = (compact) ? &ResultSetFormatterCompact : &ResultSetFormatterVerbose;
	set->recordCount = 0;
	set->column_count = 0;
	set->header_emitted = false;
	set->columns = NULL;

	set->stats.labels_added = 0;
	set->stats.nodes_created = 0;
	set->stats.properties_set = 0;
	set->stats.relationships_created = 0;
	set->stats.nodes_deleted = 0;
	set->stats.relationships_deleted = 0;

	return set;
}

void ResultSet_BuildColumns(ResultSet *set, AR_ExpNode **projections) {
	if(projections == NULL) return;

	uint projection_count = array_len(projections);
	set->column_count = projection_count;
	set->columns = array_new(const char *, projection_count);

	for(uint i = 0; i < projection_count; i ++) {
		set->columns = array_append(set->columns, projections[i]->resolved_name);
	}
}

int ResultSet_AddRecord(ResultSet *set, Record r) {
	// Prepare response arrays and emit the header if this is the first Record encountered
	if(set->header_emitted == false) _ResultSet_ReplyWithPreamble(set, r);

	set->recordCount++;

	// Output the current record using the defined formatter
	set->formatter->EmitRecord(set->ctx, set->gc, r);

	return RESULTSET_OK;
}

void ResultSet_Replay(ResultSet *set) {
	if(set->header_emitted) {
		// If we have emitted a header, set the number of elements in the preceding array.
		RedisModule_ReplySetArrayLength(set->ctx, set->recordCount);
	} else if(set->header_emitted == false && set->columns != NULL) {
		assert(set->recordCount == 0);
		// Handle the edge case in which the query was intended to return results, but none were created.
		_ResultSet_ReplyWithPreamble(set, NULL);
		RedisModule_ReplySetArrayLength(set->ctx, 0);
	} else {
		// Queries that don't emit data will only emit statistics
		RedisModule_ReplyWithArray(set->ctx, 1);
	}

	char *err = QueryCtx_GetError(); // Check to see if we've encountered a run-time error.
	if(err) RedisModule_ReplyWithError(set->ctx, err); // If so, emit it as the last top-level response.
	else _ResultSet_ReplayStats(set->ctx, set); // Otherwise, the last response is query statistics.
}

/* Report execution timing. */
void ResultSet_ReportQueryRuntime(RedisModuleCtx *ctx) {
	char *strElapsed;
	double t = QueryCtx_GetExecutionTime();
	asprintf(&strElapsed, "Query internal execution time: %.6f milliseconds", t);
	RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
	free(strElapsed);
}

void ResultSet_Free(ResultSet *set) {
	if(!set) return;

	array_free(set->columns);

	rm_free(set);
}

